#include "insertion_ordered_map.h"
#include <cassert>
#include <vector>

namespace {
    auto f(insertion_ordered_map<int, int> q)
    {
        return q;
    }
}

int main()
{
    insertion_ordered_map<int, int> m;
    for (int i = 0; i < 1500; i++) {
        m.insert(i, i);
    }

    insertion_ordered_map<int, int> m2;
    for (int i = 0; i < 1500; i++) {
        m2.insert(i + 1300, i);
    }

    m.merge(m2);

    assert(m.size() == 2800);
    assert(m.contains(2700));
    assert(m.contains(1));
    assert(!m2.contains(1004));

}

