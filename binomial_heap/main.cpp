#include "binomial_heap.h"


int main() {
    binomial_heap<int> the_heap;

    the_heap.insert(5);
    the_heap.print();

    the_heap.insert(6);
    the_heap.print();

    the_heap.clean();
    the_heap.print();
    the_heap.insert(7);
    
    the_heap.print();

    the_heap.insert(8);
    the_heap.print();

    the_heap.clean();
    the_heap.print();

    for(int k=9; k<100; ++k) {
        the_heap.insert(k);
        the_heap.print();
    }

    std::cout << std::endl;
    the_heap.clean();
    the_heap.print();
}
