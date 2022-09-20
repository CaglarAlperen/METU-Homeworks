#include <iostream>
#include <vector>
#include <chrono>

using namespace std;

// Globals
vector<int> arr;
vector<pair<int,int>> moves;
int maximum_depth = 9;
int tp;

// Functions
void handle_input()
{
    int tmp;
    while (cin >> tmp)
        arr.push_back(tmp);
}

void print_result()
{
    cout << moves.size() << endl;
    for (pair<int,int> p : moves)
    {
        cout << p.first+1 << " " << p.second+1 << endl;
    }
}

bool adjacent(int i, int j)
{
    if (i == -1)
        return arr[j] == 1;
    if (j == arr.size())
        return arr[i] == arr.size();
    else 
        return abs(arr[j] - arr[i]) == 1;
}

void reverse(int i, int j)
{
    for (int i = 0; i < arr.size(); i++)
        cout << arr[i] << " ";
    cout << endl;
    cout << "Reverse: " << i << ", " << j << endl;
    cout << "Tp before: " << tp;
    if (adjacent(i-1,i))
        tp++;
    if (adjacent(j, j+1))
        tp++;
    if (adjacent(i-1,j))
        tp--;
    if (adjacent(i, j+1))
        tp--;
    cout << " tp after: " << tp << endl;
    for (; i < j; i++, j--)
    {
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

void init_heuristic()
{
    tp = 0;

    if (arr[0] != 1)
        tp++;
    for (int i = 1; i < arr.size(); i++)
    {
        if (abs(arr[i] - arr[i-1]) != 1)
            tp++;
    }
    if (arr[arr.size()-1] != arr.size())
        tp++;
}

int heuristic()
{
    return (tp+1) / 2;
}

bool sorted()
{
    return heuristic() == 0;
}

// Depth Limited Search
bool DLS(int depth)
{
    // for (int i = 0; i < arr.size(); i++)
    //     cout << arr[i] << " ";
    // cout << endl;
    // cout << "Moves: " << moves.size() << " tp: " << tp << endl;
    if (moves.size() + heuristic() > depth)
    {
        //cout << "Depth limit exceeded" << endl;
        return false;
    }
    else if (sorted())
    {
        //cout << "Solution founded" << endl;
        return true;
    }

    for (int i = 0; i < arr.size(); i++)
    {
        for (int j = i+1; j < arr.size(); j++)
        {
            if (j == i) continue;
            //cout << "Move " << i << ", " << j << ": " << endl;
            moves.push_back(make_pair(i,j));
            reverse(i,j);
            if (DLS(depth))
                return true;
            reverse(i,j);
            moves.pop_back();
        }
    }

    return false;
}

// Depth First Iretative Deepening
void DFID()
{
    for (int depth = 0; depth <= maximum_depth; depth++)
    {
        cout << "Iteration: " << depth << endl;
        {
            Timer timer;
            init_heuristic();
            if (DLS(depth))
                return;
        }
    }
}

int main()
{
    handle_input();
    DFID();
    print_result();

    return 0;
}