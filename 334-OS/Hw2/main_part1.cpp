#include <iostream>
#include <vector>
#include <pthread.h>
#include <unistd.h>

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
    vector<pair<int,int>> areas;
    Private(int _pid, int _si, int _sj, int _tg, int _ng, vector<pair<int,int>> _areas) :
        pid(_pid), si(_si), sj(_sj), tg(_tg), ng(_ng), areas(_areas) {};
} Private;

// Globals
vector<vector<Cell>> grid;
vector<vector<pthread_mutex_t>> gridMutexes;
vector<Private> privates;
vector<pthread_t> threads;

pthread_mutex_t check_mutex;
pthread_mutex_t claim_mutex;
pthread_mutex_t wait_area_mutex;
pthread_cond_t cellUnlocked;

void *private_thread(void *vargp);
void handle_input();
void init_grid_mutexes();
void wait_area(int ai, int aj, int si, int sj, Private *p);
void claim_area(int ai, int aj, int si, int sj);
bool area_usable(int ai, int aj, int si, int sj);
void unlock_area(int ai, int aj, int si, int sj);
void gather(int ai, int aj, Private *p);

int main() 
{
    hw2_init_notifier();
    handle_input();
    init_grid_mutexes();

    for (int i = 0; i < privates.size(); i++)
    {
        if (pthread_create(&threads[i], NULL, private_thread, (void *)&privates[i]) != 0)
            cerr << "Thread create error!" << endl;
    }

    for (int i = 0; i < threads.size(); i++)
    {
        if (pthread_join(threads[i], NULL) != 0)
            cerr << "Thread join error!" << endl;
    }

    return 0;
}

void *private_thread(void *vargp)
{
    Private *prvt = (Private *)vargp;
    hw2_notify(PROPER_PRIVATE_CREATED, prvt->pid, 0, 0);
    
    for (pair<int,int> area : prvt->areas)
    {
        wait_area(area.first, area.second, prvt->si, prvt->sj, prvt);
        gather(area.first, area.second, prvt);
        unlock_area(area.first, area.second, prvt->si, prvt->sj);
    }

    hw2_notify(PROPER_PRIVATE_EXITED, prvt->pid, 0, 0);
    return NULL;
}

void gather(int ai, int aj, Private *p)
{
    hw2_notify(PROPER_PRIVATE_ARRIVED, p->pid, ai, aj);
    for (int i = ai; i < ai+p->si; i++)
    {
        for (int j = aj; j < aj+p->sj; j++)
        {
            while (grid[i][j].cigbutts > 0)
            {
                usleep(p->tg * 1000);
                grid[i][j].cigbutts--;
                hw2_notify(PROPER_PRIVATE_GATHERED, p->pid, i, j);
            }
        }
    }
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
    pthread_mutex_lock(&wait_area_mutex);
    while (!area_usable(ai, aj, si, sj))
    {
        //cout << "Private: " << p->pid << " waiting." << endl;
        pthread_cond_wait(&cellUnlocked, &wait_area_mutex);
        //cout << "Private: " << p->pid << endl;
    }
    //cout << "Private: " << p->pid << " claimed." << endl;
    claim_area(ai, aj, si, sj);
    pthread_mutex_unlock(&wait_area_mutex);
}

void claim_area(int ai, int aj, int si, int sj)
{
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
    pthread_mutex_lock(&claim_mutex);
    for (int i = ai; i < ai+si; i++)
    {
        for (int j = aj; j < aj+sj; j++)
        {
            grid[i][j].locked = false;
            pthread_cond_broadcast(&cellUnlocked);
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
    threads.resize(np);
}

void init_grid_mutexes()
{
    for (vector<pthread_mutex_t> v : gridMutexes)
        for (pthread_mutex_t mutex : v)
            pthread_mutex_init(&mutex, NULL);
}