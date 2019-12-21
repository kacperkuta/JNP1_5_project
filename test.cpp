#include <iostream>
#include <cassert>
#include "insertion_ordered_map.h"


int main() {
    insertion_ordered_map<int, int> m;
    for (int i = 0; i < 16; i++) {
        m.insert(i, 5);
    }

    for (int i = 0; i < 10; i++) {
        m.erase(i);
    }

    insertion_ordered_map<int, int> m2(m);

    m.erase(11);
    m.insert(11, 6);

    assert(m.at(11) == 6);
    assert(m2.at(11) == 5);
    m.clear();


    //std::shared_ptr<int[]> p(new int[15]);
}