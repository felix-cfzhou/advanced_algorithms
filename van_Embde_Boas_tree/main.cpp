#include "veb_tree.h"

#include <cassert>
#include <iostream>

int main() {
    veb_tree<unsigned int> a(0);

    for(unsigned int i=1; i<=1<<16; ++i) {
        a.insert(i);
        std::cout << a.predecessor(i+1).value() << std::endl;
        assert(a.predecessor(i+1).value() == i);
    }
}
