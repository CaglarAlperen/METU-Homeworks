#include <iostream>
#include <vector>
#include <algorithm>

#define N 500000

using namespace std;

typedef struct Box
{
    int w;
    int h;
    Box() : w(0), h(0) {}
    Box(int w, int h) : w(w), h(h) {}
} Box;

bool predicate(Box const& lhs, Box const& rhs)
{
    if (lhs.w != rhs.w)
        return lhs.w < rhs.w;
    return lhs.h >= rhs.h;
}

int lis(vector<int> const& a) {
    int n = a.size();
    const int INF = 2000000001;
    vector<int> d(n+1, INF);
    d[0] = -INF;

    for (int i = 0; i < n; i++) {
        int j = upper_bound(d.begin(), d.end(), a[i]) - d.begin();
        if (d[j-1] < a[i] && a[i] < d[j])
            d[j] = a[i];
    }

    int ans = 0;
    for (int i = 0; i <= n; i++) {
        if (d[i] < INF)
            ans = i;
    }
    return ans;
}

int main()
{
    int n;
    cin >> n;

    vector<Box> boxes;

    for (int i = 0; i < n; i++)
    {
        int w,h;
        cin >> w;
        cin >> h;
        boxes.push_back(Box(w,h));
    }

    sort(boxes.begin(), boxes.end(), &predicate);

    vector<int> v;

    for (int i = 0; i < n; i++)
    {
        v.push_back(boxes[i].h);
    }

    cout << lis(v);

    return 0;
}