#include <iostream>
#include <cassert>
#include <vector>
#include "insertion_ordered_map.h"


int main() {

    insertion_ordered_map<int, int> m;
    m.insert(30, 5);

    insertion_ordered_map<int, int> m2(m);

try {
//for (int i = 0; i < 13; i++)
   m.insert(3, 5);
} catch(lookup_error& e) {}


auto it = m.begin();
std::cout << it->first;

}
