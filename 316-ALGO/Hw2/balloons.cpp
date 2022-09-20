#include <iostream>
#include <vector>

using namespace std;

vector<int> PosChange;

int prize(int i, int j)
{
    if (i < j)
        return j*i*j + j*j*j;
    return i*j*i + i*i*i;
}

void pop(int k, vector<int> balloons)
{
    cout << k + PosChange[k] << " ";
    for (int i = k; i < PosChange.size(); i++)
        PosChange[i]--;
}

void popAll(int i, int j, vector<vector<int>> &S, vector<int> balloons)
{
    //cout << "PopAll: " << i << " " << j << endl;
    if (j == (i+1)%balloons.size())
        return;
    
    int k = S[i][j];
    popAll(i,k,S,balloons);
    popAll(k,j,S,balloons);
    pop(k, balloons);
}

int main()
{
    int n;
    int res = 0;
    vector<int> balloons;

    cin >> n;
    for (int i = 0; i < n; i++)
    {
        int t;
        cin >> t;
        balloons.push_back(t);
        PosChange.push_back(1);
    }
    
    int DP[n][n];
    vector<vector<int>> S(n, vector<int> (n,0));

    // init DP with -1
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
            DP[i][j] = -1;
    }

    // Step 1: calculate all DP[i][j] without popping i and j
    for (int s = 1; s < n; s++)
    {   // For subarrays size (s+1)
        for (int i = 0; i < n; i++)
        {   // For subarrays start from i
            if (s == 1)
                DP[i][(i+s)%n] = 0;
            else
            {
                // Select k between anchors provide max value
                for (int k = i+1; k < i+s; k++)
                {   // k may be bigger than n if (j < i)
                    int k_prize = DP[i][k%n] + DP[k%n][(i+s)%n] + balloons[i]*balloons[k%n]*balloons[(i+s)%n];
                    if (k_prize > DP[i][(i+s)%n])
                    {
                        DP[i][(i+s)%n] = k_prize;
                        S[i][(i+s)%n] = k%n;
                    }
                }
            }
        }
    }

    // for (int i = 0; i < n; i++)
    // {
    //     for (int j = 0; j < n; j++)
    //     {
    //         cout << DP[i][j] << " ";
    //     }
    //     cout << endl;
    // }

    int res_i, res_j;
    // Step 2: add bursting prizes of anchors
    for (int i = 0; i < n; i++)
    {
        for (int j = i+1; j < n; j++)
        {
            DP[i][j] += DP[j][i] + prize(balloons[i],balloons[j]);
            if (DP[i][j] > res)
            {
                res = DP[i][j];
                res_i = i;
                res_j = j;
            }
        }
    }

    //cout << "Res i,j: " << res_i << " " << res_j << endl;

    // Print DP
    // for (int i = 0; i < n; i++)
    // {
    //     for (int j = 0; j < n; j++)
    //     {
    //         cout << DP[i][j] << " ";
    //         if (DP[i][j] > res)
    //             res = DP[i][j];
    //     }
    //     cout << endl;
    // }

    cout << "S vec: " << endl;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            cout << S[i][j] << " ";
        }
        cout << endl;
    }
    
    //cout << "Result: " << res << endl;


    cout << res << endl;
    popAll(res_i,res_j,S,balloons);
    popAll(res_j,res_i,S,balloons);

    if (balloons[res_i] < balloons[res_j])
    {
        pop(res_i, balloons);
        pop(res_j, balloons);
    }
    else
    {
        pop(res_j, balloons);
        pop(res_i, balloons);
    }

    return 0;
}