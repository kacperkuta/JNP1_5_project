#include <iostream>
#include <cassert>
#include "insertion_ordered_map.h"


int main() {
    insertion_ordered_map<int, int> m;
   for (int i = 0; i < 13; i++) {
       m.insert(i, 6);

       //assert(!m.contains(i));

   }
    m.erase(1);
   m.erase(5);
   m.erase(12);
m.erase(0);

    for (auto itr = m.begin(); itr != m.end(); ++itr) {
        std::cout << (*itr).first << std::endl;
    }
/*
   m.insert(0, 5);
    auto itr = m.begin();
    std::cout << (*itr).first;
    */
}