#ifndef VEB_TREE_H
#define VEB_TREE_H

#include <type_traits>
#include <unordered_map>
#include <memory>
#include <optional>
#include <iostream>
#include <bitset>


template<typename T, typename Enable=void> class veb_tree;

template<typename T> class veb_tree<T, typename std::enable_if<std::is_integral_v<T> && !std::is_signed_v<T>>::type> {
    private:
        static constexpr size_t range = sizeof(T) * 8;

        struct veb_tree_node {
            T minimum, maximum;

            veb_tree_node(T item):
                minimum{item},
                maximum{item}
            {}

            virtual std::optional<T> predecessor(T x) const = 0;
            virtual void insert(T x) = 0;
            virtual bool remove(T x) = 0;

            virtual ~veb_tree_node() {};
        };

        struct veb_tree_node_small: public veb_tree_node {
            // only 8 bits
            static constexpr size_t range = 256;

            std::bitset<range> the_node;

            veb_tree_node_small(T item):
                veb_tree_node{item},
                the_node{}
            {
                std::cout << "creating " << std::bitset<8>(item) << " into small node" << std::endl;
                the_node[item] = true;
            }

            std::optional<T> predecessor(T x) const override {
                while(x-- > 0) {
                    if(the_node[x]) return x;
                }

                return {};
            }

            void insert(T x) override {
                std::cout << "inserting " << std::bitset<8>(x) << " into small node min max " << std::bitset<8>(this->minimum) << ',' << std::bitset<8>(this->maximum) << std::endl;
                if(x < this->minimum) {
                    this->minimum = x;
                }
                else if(x > this->maximum) {
                    this->maximum = x;
                }

                the_node[x] = true;
            }

            bool remove(T x) override {
                if(!the_node[x]) return false;
                else if(this->minimum == this->maximum) return true;

                the_node[x] = false;

                if(x == this->minimum) {
                    while(x++ < range) {
                        if(the_node[x]) {
                            this->minimum = x;
                            break;
                        }
                    }
                }
                else if(x == this->maximum) {
                    while(x-- > 0) {
                        if(the_node[x]) {
                            this->maximum = x;
                            break;
                        }
                    }
                }

                return false;
            }

            ~veb_tree_node_small() {}
        };

        struct veb_tree_node_large: public veb_tree_node {
            const size_t num_bits;
            const T half_mask;

            std::unordered_map<T, std::unique_ptr<veb_tree_node>> clusters;
            std::unique_ptr<veb_tree_node> summary;

            veb_tree_node_large(size_t num_bits, T item):
                veb_tree_node{item},
                num_bits{num_bits},
                half_mask{(T(1) << num_bits/2) - 1},
                clusters{},
                summary{}
            {
                std::cout << "creating " << std::bitset<range>(item) << " into large node of range " << num_bits << std::endl;
            }

            std::optional<T> predecessor(T x) const override {
                const auto c = cluster_of(x);
                const auto i = id_of(x);

                if(x <= this->minimum) return {};
                else if(auto it = clusters.find(c); it!=clusters.end() && i>it->second->minimum) {
                    return combine(c, it->second->predecessor(i).value()); 
                }
                else {
                    if(summary) {
                        const auto bigger_cluster = summary->predecessor(c).value();
                        return combine(bigger_cluster, clusters.at(bigger_cluster)->maximum);
                    }
                    else {
                        return this->minimum;
                    }
                }
            }

            void insert(T x) override {
                std::cout << "inserting " << std::bitset<range>(x) << " into large node of range " << num_bits << " min max: " << std::bitset<range>(this->minimum) << ',' << std::bitset<range>(this->maximum) << std::endl;
                if(x < this->minimum) {
                    std::swap(this->minimum, x);
                }
                else if(x > this->maximum) {
                    this->maximum = x;
                }

                const auto c = cluster_of(x);
                const auto i = id_of(x);

                if(auto it = clusters.find(c); it == clusters.end()) {
                    std::cout << "not seen " << std::bitset<range>(x) << " in large node of range " << num_bits << std::endl;

                    if(!summary) {
                        if(num_bits > 16) {
                            summary = std::make_unique<veb_tree_node_large>(num_bits/2, c);
                        }
                        else {
                            summary = std::make_unique<veb_tree_node_small>(c);
                        }
                    }
                    else{
                        summary->insert(c);
                    }

                    if(num_bits > 16) {
                        clusters.emplace(c, std::make_unique<veb_tree_node_large>(num_bits/2, i));
                    }
                    else {
                        clusters.emplace(c, std::make_unique<veb_tree_node_small>(i));
                    }
                }
                else {
                    it->second->insert(i);
                }
            }

            bool remove(T x) override {
                const auto c = cluster_of(x);
                const auto i = id_of(x);

                if(x == this->minimum) {
                    if(this->minimum == this->maximum) return true;
                    else if(auto it = clusters.find(summary->minimum); it != clusters.end()) {
                        this->minimum = combine(summary->minimum, it->second->minimum);
                        if(it->second->remove(it->second->minimum)) {
                            clusters.erase(it);
                            if(summary->remove(summary->minimum)) summary.reset();
                        }
                    }
                }
                else {
                    if(auto it = clusters.find(c); it != clusters.end()) {
                        if(it->second->remove(i)){
                            clusters.erase(it);
                            if(summary->remove(c)) summary.reset();
                        }

                        if(x == this->maximum) {
                            if(summary) this->maximum = combine(summary->maximum, clusters.at(summary->maximum)->maximum);
                            else this->maximum = this->minimum;
                        }
                    }
                }


                return false;
            }

            ~veb_tree_node_large() {}

            private:
            T cluster_of(T item) const { return item >> num_bits/2; }
            T id_of(T item) const { return item & half_mask; }
            T combine(T c, T i) const { return c << num_bits/2 | i; }
        };

        std::unique_ptr<veb_tree_node> the_tree;

    public:
        veb_tree(T x):
            the_tree{
                range > 16 ?
                    static_cast<std::unique_ptr<veb_tree_node>>(std::make_unique<veb_tree_node_large>(range, x)) :
                    static_cast<std::unique_ptr<veb_tree_node>>(std::make_unique<veb_tree_node_small>(x))
            }
        {}
        veb_tree() {}

        std::optional<T> predecessor(T x) const {
            if(the_tree) return the_tree->predecessor(x);
            else return {};
        }

        void insert(T x) {
            if(the_tree) the_tree->insert(x);
            else if(range > 16) the_tree = std::make_unique<veb_tree_node_large>(range, x);
            else the_tree = std::make_unique<veb_tree_node_small>(x);
        }

        void remove(T x) {
            if(the_tree && the_tree->remove(x)) the_tree.reset();
        }
};

#endif
