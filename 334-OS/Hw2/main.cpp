#include <iostream>
#include <vector>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#include "hw2_output.h"

using namespace std;

typedef struct Cell
{
    int cigbutts;
    bool locked;
} Cell;

typedef struct Private
{
    int pid;
    int si, sj;
    int tg;
    int ng;
    int cleared_areas;
    vector<pair<int,int>> areas;
    Private(int _pid, int _si, int _sj, int _tg, int _ng, vector<pair<int,int>> _areas) :
        pid(_pid), si(_si), sj(_sj), tg(_tg), ng(_ng), areas(_areas), cleared_areas(0) {};
} Private;

typedef struct Order
{
    int time;
    string type;
} Order;

// Globals
vector<vector<Cell>> grid;
vector<vector<pthread_mutex_t>> gridMutexes;
vector<Private> privates;
vector<Order> orders;
vector<pthread_t> private_threads;
vector<pthread_t> order_threads;

pthread_mutex_t check_mutex;
pthread_mutex_t claim_mutex;
pthread_mutex_t wait_area_mutex;
pthread_mutex_t order_mutex;
pthread_mutex_t wait_response_mutex;
pthread_mutex_t wait_continue_mutex;
pthread_mutex_t gather_mutex;

pthread_cond_t cell_unlocked;
pthread_cond_t order_given;
pthread_cond_t order_taken;

bool working = true;
bool stop = false;
int active_worker = 0;
int exited_private = 0;

void *private_thread(void *vargp);
void *order_thread(void *vargp);
void handle_input();
void init_grid_mutexes();
void wait_area(int ai, int aj, int si, int sj, Private *p);
void claim_area(int ai, int aj, int si, int sj);
bool area_usable(int ai, int aj, int si, int sj);
void unlock_area(int ai, int aj, int si, int sj);
void gather(int ai, int aj, Private *p);
void create_threads();
void join_threads();
bool meaningful_order(Order ord);
void wait_for_everyone_to_break();
void wait_for_everyone_to_continue();
void wait_for_everyone_to_stop();
struct timespec get_wait_time(int ms);

int main() 
{
    handle_input();
    init_grid_mutexes();

    hw2_init_notifier();
    create_threads(); 
    join_threads();    

    return 0;
}

void *private_thread(void *vargp)
{
    Private *prvt = (Private *)vargp;
    hw2_notify(PROPER_PRIVATE_CREATED, prvt->pid, 0, 0);
    
    for (int i = 0; i < prvt->areas.size(); i=prvt->cleared_areas)
    {
        //cout << "i: " << i << " total_areas: " << prvt->areas.size() << endl;
        wait_area(prvt->areas[i].first, prvt->areas[i].second, prvt->si, prvt->sj, prvt);
        gather(prvt->areas[i].first, prvt->areas[i].second, prvt);
        unlock_area(prvt->areas[i].first, prvt->areas[i].second, prvt->si, prvt->sj);
        if (stop)
        {
            hw2_notify(PROPER_PRIVATE_STOPPED, prvt->pid, 0, 0);
            exited_private++;
            pthread_cond_broadcast(&order_taken);
            hw2_notify(PROPER_PRIVATE_EXITED, prvt->pid, 0, 0);
            return NULL;
        }
        pthread_mutex_lock(&wait_continue_mutex);
        while (!working)
        {
            //cout << "Not working!" << endl;
            pthread_cond_wait(&order_given, &wait_continue_mutex);
            if (stop)
            {
                // STOP order is given
                hw2_notify(PROPER_PRIVATE_STOPPED, prvt->pid, 0, 0);
                exited_private++;
                pthread_cond_broadcast(&order_taken);
                hw2_notify(PROPER_PRIVATE_EXITED, prvt->pid, 0, 0);
                return NULL;
            }
            if (working) 
            {
                hw2_notify(PROPER_PRIVATE_CONTINUED, prvt->pid, 0, 0);
                //cout << prvt->pid << " Continue!" << endl;
                active_worker++;
                pthread_cond_broadcast(&order_taken);
            }
        }
        pthread_mutex_unlock(&wait_continue_mutex);
    }

    exited_private++;
    hw2_notify(PROPER_PRIVATE_EXITED, prvt->pid, 0, 0);
    return NULL;
}

void *order_thread(void *vargp)
{
    Order *ord = (Order *)vargp;

    usleep(ord->time * 1000);

    if (ord->type == "break") hw2_notify(ORDER_BREAK, 0, 0, 0);
    else if (ord->type == "continue") hw2_notify(ORDER_CONTINUE, 0, 0, 0);
    else hw2_notify(ORDER_STOP, 0, 0, 0);
    
    if (meaningful_order(*ord))
    {
        pthread_cond_broadcast(&order_given);
        pthread_mutex_lock(&order_mutex);
        if (ord->type == "break")
        {
            working = false;
            wait_for_everyone_to_break();
        }
        else if (ord->type == "continue")
        {
            working = true;
            wait_for_everyone_to_continue();
        }
        else if (ord->type == "stop")
        {
            stop = true;
            wait_for_everyone_to_stop();
        }
        pthread_mutex_unlock(&order_mutex);
    }

    //cout << "Order thread finished" << endl;
    return NULL;
}

void wait_for_everyone_to_break()
{
    //cout << "Waiting for everyone to break..." << endl;
    //cout << "active worker: " << active_worker << endl;
    pthread_mutex_lock(&wait_response_mutex);
    while (active_worker > 0)
    {
        pthread_cond_wait(&order_taken, &wait_response_mutex);
        //cout << "active_worker: " << active_worker << endl;
    }
    pthread_mutex_unlock(&wait_response_mutex);
    //cout << "Everyone gave break!" << endl;
}

void wait_for_everyone_to_continue()
{
    //cout << "Waiting for everyone to continue..." << endl;
    //cout << "active worker: " << active_worker << endl; 
    pthread_mutex_lock(&wait_response_mutex);
    while (active_worker < privates.size())
    {
        pthread_cond_wait(&order_taken, &wait_response_mutex);
    }
    pthread_mutex_unlock(&wait_response_mutex);
    //cout << "Everyone is continuing!" << endl;
}

void wait_for_everyone_to_stop()
{
    pthread_mutex_lock(&wait_response_mutex);
    while (exited_private < privates.size())
    {
        pthread_cond_wait(&order_taken, &wait_response_mutex);
    }
    pthread_mutex_unlock(&wait_response_mutex);
    //cout << "Order stop finished" << endl;
}

void gather(int ai, int aj, Private *p)
{
    if (stop)
    {
        // STOP order is given
        //hw2_notify(PROPER_PRIVATE_STOPPED, p->pid, 0, 0);
        //cout << p->pid << " Stopping..." << endl;
        active_worker--;
        //pthread_cond_broadcast(&order_taken);
        return;
    }
    else if (!working)
    {
        // BREAK order is given
        //cout << p->pid << " Giving break..." << endl;
        hw2_notify(PROPER_PRIVATE_TOOK_BREAK, p->pid, 0, 0);
        active_worker--;
        pthread_cond_broadcast(&order_taken);
        return;
    }

    hw2_notify(PROPER_PRIVATE_ARRIVED, p->pid, ai, aj);
    for (int i = ai; i < ai+p->si; i++)
    {
        for (int j = aj; j < aj+p->sj; j++)
        {
            while (grid[i][j].cigbutts > 0)
            {
                int err;
                //cout << "Before gather mutex" << endl;
                pthread_mutex_lock(&gather_mutex);
                //cout << "After gather mutex" << endl;
                while (!stop && working)
                {
                    //cout << "Waiting time to gather" << endl;
                    struct timespec time_to_wait = get_wait_time(p->tg);
                    err = pthread_cond_timedwait(&order_given, &gather_mutex, &time_to_wait);
                    if (err == ETIMEDOUT)
                    {
                        //cout << "Time is up. Gathering..." << endl;
                        if (stop)
                        {
                            // STOP order is given
                            //hw2_notify(PROPER_PRIVATE_STOPPED, p->pid, 0, 0);
                            //cout << p->pid << " Stopping..." << endl;
                            active_worker--;
                            //pthread_cond_broadcast(&order_taken);
                            pthread_mutex_unlock(&gather_mutex);
                            return;
                        }
                        else if (!working)
                        {
                            // BREAK order is given
                            //cout << p->pid << " Giving break..." << endl;
                            hw2_notify(PROPER_PRIVATE_TOOK_BREAK, p->pid, 0, 0);
                            active_worker--;
                            pthread_cond_broadcast(&order_taken);
                            pthread_mutex_unlock(&gather_mutex);
                            return;
                        }
                        grid[i][j].cigbutts--;
                        hw2_notify(PROPER_PRIVATE_GATHERED, p->pid, i, j);
                        break;
                    }
                    else
                    {
                        if (stop)
                        {
                            // STOP order is given
                            //hw2_notify(PROPER_PRIVATE_STOPPED, p->pid, 0, 0);
                            //cout << p->pid << " Stopping..." << endl;
                            active_worker--;
                            //pthread_cond_broadcast(&order_taken);
                            pthread_mutex_unlock(&gather_mutex);
                            return;
                        }
                        else if (!working)
                        {
                            // BREAK order is given
                            //cout << p->pid << " Giving break..." << endl;
                            hw2_notify(PROPER_PRIVATE_TOOK_BREAK, p->pid, 0, 0);
                            active_worker--;
                            pthread_cond_broadcast(&order_taken);
                            pthread_mutex_unlock(&gather_mutex);
                            return;
                        }
                    }
                }
                pthread_mutex_unlock(&gather_mutex);
            }
        }
    }
    p->cleared_areas++;
    hw2_notify(PROPER_PRIVATE_CLEARED, p->pid, 0, 0);
}

bool area_usable(int ai, int aj, int si, int sj)
{
    pthread_mutex_lock(&check_mutex);
    for (int i = ai; i < ai+si; i++)
    {
        for (int j = aj; j < aj+sj; j++)
        {
            if (grid[i][j].locked == true)
            {
                pthread_mutex_unlock(&check_mutex);
                return false;
            }
        }
    }
    pthread_mutex_unlock(&check_mutex);
    return true;
}

void wait_area(int ai, int aj, int si, int sj, Private *p)
{
    //cout << "Waiting for area..." << endl;
    pthread_mutex_lock(&wait_area_mutex);
    while (!area_usable(ai, aj, si, sj))
    {
        pthread_cond_wait(&cell_unlocked, &wait_area_mutex);
    }
    claim_area(ai, aj, si, sj);
    pthread_mutex_unlock(&wait_area_mutex);
}

void claim_area(int ai, int aj, int si, int sj)
{
    //cout << "Claiming area..." << endl;
    pthread_mutex_lock(&claim_mutex);
    for (int i = ai; i < ai+si; i++)
    {
        for (int j = aj; j < aj+sj; j++)
        {
            grid[i][j].locked = true;
        }
    }
    pthread_mutex_unlock(&claim_mutex);
}

void unlock_area(int ai, int aj, int si, int sj)
{
    //cout << "Unlocking area..." << endl;
    pthread_mutex_lock(&claim_mutex);
    for (int i = ai; i < ai+si; i++)
    {
        for (int j = aj; j < aj+sj; j++)
        {
            grid[i][j].locked = false;
            pthread_cond_broadcast(&cell_unlocked);
        }
    }
    pthread_mutex_unlock(&claim_mutex);
}

void handle_input()
{
    int n, m;
    cin >> n >> m;
    // Get the size of the grid
    grid.resize(n, vector<Cell>(m));
    gridMutexes.resize(n, vector<pthread_mutex_t>(m));

    // Place cigbutt numbers
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            cin >> grid[i][j].cigbutts;
    
    // Create privates
    int np; // number of privates
    cin >> np;
    for (int i = 0; i < np; i++)
    {
        int pid, si, sj, tg, ng;
        cin >> pid >> si >> sj >> tg >> ng;
        vector<pair<int,int>> areas;
        for (int j = 0; j < ng; j++)
        {
            int ai, aj;
            cin >> ai >> aj;
            areas.push_back(make_pair(ai, aj));
        }
        Private p(pid, si, sj, tg, ng, areas);
        privates.push_back(p);
    }
    private_threads.resize(np);
    active_worker = np;

    // If there is any order
    int norder;
    if (cin >> norder)
    {
        orders.resize(norder);
        for (int i = 0; i < norder; i++)
        {
            cin >> orders[i].time >> orders[i].type;
        }
        order_threads.resize(norder);
    }
}

void init_grid_mutexes()
{
    for (vector<pthread_mutex_t> v : gridMutexes)
        for (pthread_mutex_t mutex : v)
            pthread_mutex_init(&mutex, NULL);
}

void create_threads()
{
    for (int i = 0; i < private_threads.size(); i++)
    {
        if (pthread_create(&private_threads[i], NULL, private_thread, (void *)&privates[i]) != 0)
            cerr << "Thread create error!" << endl;
    }

    for (int i = 0; i < order_threads.size(); i++)
    {
        if (pthread_create(&order_threads[i], NULL, order_thread, (void *)&orders[i]) != 0)
            cerr << "Thread create error!" << endl;
    }
}

void join_threads()
{
    for (int i = 0; i < private_threads.size(); i++)
    {
        if (pthread_join(private_threads[i], NULL) != 0)
            cerr << "Thread join error!" << endl;
    }

    for (int i = 0; i < order_threads.size(); i++)
    {
        if (pthread_join(order_threads[i], NULL) != 0)
            cerr << "Thread join error!" << endl;
    }
}

bool meaningful_order(Order ord)
{
    if (ord.type == "break")
        return !stop && working;
    if (ord.type == "continue")
        return !stop && !working;
    else 
        return !stop;
}

struct timespec get_wait_time(int ms)
{
    struct timespec time_to_wait;
    struct timeval now;

    gettimeofday(&now, NULL);

    time_to_wait.tv_sec = now.tv_sec + ms/1000;
    time_to_wait.tv_nsec = (now.tv_usec + (ms%1000)*1000)*1000;

    return time_to_wait;
}
