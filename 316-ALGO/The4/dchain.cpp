#include <iostream>
#include <vector>

using namespace std;

vector<int> chain;
int target;

void handle_input()
{
    cin >> target;
}

int h()
{
    int h = 0;
    int v = chain.back();
    int prev = chain[chain.size()-2];

    while (v < target)
    {
        int sum = v + prev;
        prev = v;
        v = sum;
        h++;
    }

    return h;
}

bool DLS(int depth)
{
    if (chain.size() + h() > depth)
    {
        return false;
    }
    else if (chain.back() == target)
    {
        return true;
    }

    for (int i = chain.size()-1; i >= 0; i--)
    {
        for (int j = i-1; j >= 0; j--)
        {
            int v = chain[i] + chain[j];
            if (chain.back() < v && v <= target)
            {
                chain.push_back(v);
                if (DLS(depth))
                {
                    return true;
                }
                chain.pop_back();
            }
        }
    }

    return false;
}

void DFID()
{
    chain = {1, 2};
    for (int depth = 2; !DLS(depth) ; depth++);
}

void print_result()
{
    for (int i : chain)
        cout << i << " ";
    cout << "\n";
}

int main()
{
    handle_input();
    DFID();
    print_result();

    return 0;
}