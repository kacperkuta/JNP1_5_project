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
    insertion_ordered_map<int, int> m1;
    insertion_ordered_map<int, int> m2(m1);
    insertion_ordered_map<int, int> m3(m2);
    insertion_ordered_map<int, int> m4(m1);

    assert(m1.begin() == m2.begin());
    assert(m2.begin() == m4.begin());
    assert(m1.begin() == m3.begin());

    m1.insert(4, 5);
    m1.insert(3, 2);

    assert(m2.begin() == m4.begin());
    assert(m3.begin() == m4.begin());
    assert(m1.contains(4));
    assert(!m2.contains(4));
    assert((!m3.contains(3)));

    m2.merge(m1);

    assert(m2.contains(3));
    assert(m2.contains(4));
    assert((!m3.contains(3)));
    assert((!m4.contains(4)));

    m2.insert(7, 7);
    assert(m2.contains(7));
    assert(!m1.contains(7));

}

