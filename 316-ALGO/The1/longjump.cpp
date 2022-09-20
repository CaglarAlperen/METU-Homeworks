#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>

using namespace std;

int lsb(int i)
{
    return i & (i ^ (i - 1));
}

void update(vector<int>& Fw, int n)
{
    int size = Fw.size();
    for (int i = n; i < size; i += lsb(i))
    {
        Fw[i]++;
    }
}

int query(vector<int>& Fw, int n)
{
    int size = Fw.size();
    int res = 0;
    for (int i = n; i > 0; i -= lsb(i))
    {
        res += Fw[i];
    }
    return res;
}

int main()
{
    int n,t,r = 1;
    cin >> n;
    
    vector<pair<int, int>> scores;
    vector<pair<int, int>> scores_cp;
    vector<int> res;
    
    for (int i = 0; i < n; i++)
    {
        cin >> t;
        scores.push_back(make_pair(t,0));
        scores_cp.push_back(make_pair(t,i));
    }
    
    sort(scores_cp.begin(), scores_cp.end());

    cout << "Scores_cp" << endl;
    for (int i = 0; i < n; i++)
        cout << scores_cp[i].first << " " << scores_cp[i].second << ", ";
    
    for (int i = 0; i < n; i++)
    {
        scores[scores_cp[i].second].second = r;
        if (i == n-1)
            break;
        if (scores_cp[i].first != scores_cp[i+1].first)
            r++;
    }

    cout << endl;
    cout << "Scores" << endl;
    for (int i = 0; i < n; i++)
        cout << scores[i].first << " " << scores[i].second << ", ";
    
    vector<int> Fw;
    for (int i = 0; i <= r; i++)
        Fw.push_back(0);

    for (int i = 0; i < n; i++)
    {
        res.push_back(query(Fw, r) - query(Fw, scores[i].second - 1));
        update(Fw, scores[i].second);

        cout << "Fw state after " << i << ": " << endl;
        for (int h = 1; h <= r; h++)
            cout << Fw[h] << " ";
        cout << endl;
    }
    
    for (int i = 0; i < n; i++)
        cout << res[i]+1 << " ";
    
    return 0;
}