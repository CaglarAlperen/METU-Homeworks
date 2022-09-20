#include <iostream>
#include <cstring>
#include <vector>

using namespace std;

void printIndexes(vector<vector<vector<int>>>& S, int i, int j)
{
    if (j == i+1)
        return;
    if (j == i+4)
    {
        cout << i+2 << " ";
        return;
    }
    
    printIndexes(S, i, S[i][j][0]);
    printIndexes(S, S[i][j][0], S[i][j][1]);
    printIndexes(S, S[i][j][1], S[i][j][2]);
    printIndexes(S, S[i][j][2], j);

    cout << S[i][j][1] << " ";
}

int main()
{
    int n;
    cin >> n;
    int balloons[n+2];
    for (int i = 1; i < n+1; i++)
        cin >> balloons[i];

    vector<vector<int>> DP(n+2, vector<int> (n+2,0));
    vector<vector<vector<int>>> S(n+2, vector<vector<int>> (n+2, vector<int>(3,0)));
    //memset(DP, 0, (n+2)*(n+2)*sizeof(int));

    //cout << "S size: " << S.size() << " " << S[0].size() << " " << S[0][0].size() << endl; 

    for (int s = 1; s < n+2; s+=3)
    {
        for (int i = 0; i+s < n+2; i++)
        {
            if (s == 1)
            {
                DP[i][i+s] = 0;
            }
            if (s == 4)
            {
                DP[i][i+s] = balloons[i+1]*balloons[i+2]*balloons[i+3];
            }
            else 
            {
                for (int k1 = i+1; k1 < i+s; k1++)
                {
                    for (int k2 = k1+1; k2 < i+s; k2+=3)
                    {
                        if (k2 == k1) continue;
                        for (int k3 = k2+1; k3 < i+s; k3+=3)
                        {
                            if (k3 == k2 || k3 == k1) continue;
                            int k_val = DP[i][k1] + DP[k1][k2] + DP[k2][k3] + DP[k3][i+s]
                                        + balloons[k1]*balloons[k2]*balloons[k3];
                            if (k_val > DP[i][i+s])
                            {
                                DP[i][i+s] = k_val;
                                S[i][i+s] = vector<int>{k1,k2,k3};
                            }
                        }
                    }
                }
            }
        }
    }
    

    // cout << "DP" << endl;
    // for (int i = 0; i < n+2; i++)
    // {
    //     for (int j = 0; j < n+2; j++)
    //     {
    //         cout << DP[i][j] << " ";
    //     }
    //     cout << endl;
    // }

    //cout << "Indexes: " << endl;

    cout << DP[0][n+1] << endl;
    printIndexes(S, 0, n+1);
    cout << endl;

    return 0;
}