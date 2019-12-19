#include <iostream>
#include "insertion_ordered_map.h"

class Liczba {
public:
    int a;

    Liczba(int aa) : a(aa) {};
};

int main() {
    std::shared_ptr<Liczba> a(new Liczba(14));
    Liczba* c = new Liczba(13);
    a.reset(c);

}