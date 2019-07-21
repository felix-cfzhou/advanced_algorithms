#include "bloom_filter.h"

#include <cassert>
#include <iostream>


std::hash<size_t> the_hasher{};

size_t identity(const size_t &key) { return key; }
size_t std_hash(const size_t &key) { return the_hasher(key); }


int main() {
    bloom_filter<size_t, std_hash, identity> filter;

    for(size_t k=0; k<0x0000FFFF; ++k) {
        filter.insert(k);
        assert(filter.contains(k));
        assert(filter.contains(0));
        std::cout << "inserting " << std::bitset<64>(k) << std::endl;
    }
}
