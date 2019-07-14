#include "veb_tree.h"

#include <iostream>

int main() {
    veb_tree<unsigned int> a(0);
    for(unsigned int i=1; i<1<<16; ++i) {
        a.the_tree.insert(i);
        a.the_tree.predicate(i+1);
    }
}
