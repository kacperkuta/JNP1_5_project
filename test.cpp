#include <iostream>
#include <cassert>
#include "insertion_ordered_map.h"


int main() {
    insertion_ordered_map<int, int> m;

    for (int i = 0; i < 15; i++) {
        m.insert(i, 55);
    }

    int& v = m.at(44);

    insertion_ordered_map<int, int> m2(m);

    v = 10;

    assert(m[4] == 10);
    assert(m2[4] == 55);

}