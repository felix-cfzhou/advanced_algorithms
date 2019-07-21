#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER_H

#include <functional>
#include <bitset>
#include <limits>


template<typename T> using HashFunc = size_t(const T&);

template<
    typename T,
    HashFunc<T> h1,
    HashFunc<T> h2,
    size_t num_filters = 16,
    size_t filter_size = std::numeric_limits<int16_t>::max()
> class bloom_filter {
    std::array<std::bitset<filter_size>, num_filters> the_filters;

    size_t multiplicative_hash(const T& key, size_t func_num) const {
        return (h1(key) + func_num*h2(key)) % filter_size;
    }

    public:
    bloom_filter():
        the_filters{}
    {}

    void insert(const T& key) {
        for(size_t k=0; k<num_filters; ++k) {
            the_filters.at(k)[multiplicative_hash(key, k)] = true;
        }
    }

    bool contains(const T& key) const {
        for(size_t k=0; k<num_filters; ++k) {
            if(!(the_filters.at(k)[multiplicative_hash(key, k)])) return false;
        }

        return true;
    }
};

#endif
