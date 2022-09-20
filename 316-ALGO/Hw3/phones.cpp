#include <iostream>
#include <set>

using namespace std;

typedef enum
{
    LEFT_EDGE = 0,
    POINT,
    RIGHT_EDGE
} EventType;

typedef struct Event
{
    EventType type;
    int x;
    int y;
    Event(EventType _type, int _x, int _y) : 
        type(_type), x(_x), y(_y) {};
} Event;

auto cmp = [](Event a, Event b) {
    if (a.x == b.x)
    {
        if (a.type == b.type)
            return a.y < b.y;
        return a.type < b.type;
    }
    return a.x < b.x;
};

multiset<Event, decltype(cmp)> events(cmp);

int main()
{
    int n, m;
    cin >> n;
    for (int i = 0; i < n; i++)
    {
        int l, h, r;
        cin >> l >> h >> r;
        events.insert(Event(EventType::LEFT_EDGE, l, h));
        events.insert(Event(EventType::RIGHT_EDGE, r, h));
    }

    cin >> m;
    for (int i = 0; i < m; i++)
    {
        int x, y;
        cin >> x >> y;
        events.insert(Event(EventType::POINT, x, y));
    }

    multiset<int> heights;
    int res = 0;

    for (Event e : events)
    {
        if (e.type == EventType::LEFT_EDGE)
        {
            heights.insert(e.y);
        }
        else if (e.type == EventType::POINT)
        {
            if (heights.size() == 0)
                res++;
            else if (e.y > *heights.rbegin())
                res++;
        }
        else if (e.type == EventType::RIGHT_EDGE)
        {
            heights.erase(heights.lower_bound(e.y));
        }
    }
    cout << res << endl;

    return 0;
}