#ifndef BINOMIAL_HEAP_H
#define BINOMIAL_HEAP_H

#include <vector>
#include <memory>
#include <list>
#include <optional>
#include <iostream>


template<typename T, typename Compare = std::less<T>> class binomial_heap {
    struct binomial_heap_node {
        using children_t = typename std::vector<std::unique_ptr<binomial_heap_node>>;

        size_t rank;
        T key;
        children_t children;

        template<typename X=T> binomial_heap_node(X &&key, children_t children):
            rank{0},
            key{std::forward<X>(key)},
            children{std::move(children)}
        {}

        template<typename X> void merge(X &&other) {
            children.emplace_back(std::forward<X>(other));

            ++rank;
        }

        void print(size_t depth=0) const {
            std::cout << '(' << rank << ", " << key << ") " << std::endl;
            for(auto &it : children) {
                for(size_t k=0; k<=depth; ++k) std::cout << "  ";
                it->print(depth+1);
            }
        }
    };

    using node_t = typename std::unique_ptr<binomial_heap_node>;

    Compare comp;
    std::list<node_t> the_binomial_tree;
    std::optional<T> the_max_or_min;

    node_t merge(node_t first, node_t second) const {
        if(comp(first->key, second->key)) {
            first->merge(second.release());

            return first;
        }
        else {
            second->merge(first.release());

            return second;
        }
    }

    auto get_target_it() {
        auto target_it = the_binomial_tree.front();
        for(auto it = ++the_binomial_tree.begin(); it!=the_binomial_tree.end(); ++it) {
            if(comp(it->key, target_it->key)) target_it = it;
        }

        return target_it;
    }

    public:
    template<typename X=T> void insert(X &&key) {
        the_binomial_tree.emplace_back(
                std::make_unique<binomial_heap_node>(
                    std::forward<X>(key), typename binomial_heap_node::children_t()
                    )
                );
    }

    std::optional<T> max_or_min() const {
        return the_max_or_min;
    }

    // assumes non empty
    void clean() {
        std::vector<node_t> forest;

        for(node_t current; current || !the_binomial_tree.empty();) {
            if(!current) {
                current = std::move(the_binomial_tree.back());
                the_binomial_tree.pop_back();
            }

            while(forest.size() <= current->rank) forest.emplace_back();
            if(forest.at(current->rank)) {
                node_t other = std::move(forest.at(current->rank));
                current = merge(std::move(current), std::move(other));
            }
            else {
                forest[current->rank] = std::move(current);
            }
        }

        for(size_t k=forest.size(); k>0; --k) {
            if(forest.at(k-1)) {
                the_binomial_tree.emplace_back(std::move(forest.at(k-1)));
            }
        }
    }

    void delete_max_or_min() {
        if(the_binomial_tree.empty()) return;

        auto target_it = get_target_it(); 
        for(size_t k=0; k<target_it->children.size(); ++k) {
            the_binomial_tree.emplace_back(std::move(target_it->children.at(k)));
        }

        clean();

        the_binomial_tree.erase(target_it);
        if(the_binomial_tree.empty()) the_max_or_min = {};
        else the_max_or_min = get_target_it()->key;
    }

    void print() const {
        for(auto &it : the_binomial_tree) it->print();
    }
};

#endif
