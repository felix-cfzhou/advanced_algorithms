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
    public:
        static constexpr size_t range = sizeof(T) * 8;

        struct veb_tree_node {
            T minimum, maximum;

            veb_tree_node(T item):
                minimum{item},
                maximum{item}
            {}

            virtual std::optional<T> predicate(T x) const = 0;
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

            std::optional<T> predicate(T x) const override {
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
                summary{
                    std::move(
                            num_bits > 16 ?
                            static_cast<std::unique_ptr<veb_tree_node>>(
                                std::make_unique<veb_tree_node_large>(num_bits/2, cluster_of(item))
                                ) :
                            static_cast<std::unique_ptr<veb_tree_node>>(
                                std::make_unique<veb_tree_node_small>(cluster_of(item))
                                )
                            )
                }
            {
                std::cout << "creating " << std::bitset<range>(item) << " into large node of range " << num_bits << std::endl;
                if(num_bits > 16) {
                    clusters.emplace(cluster_of(item), std::make_unique<veb_tree_node_large>(num_bits/2, id_of(item)));
                }
                else {
                    clusters.emplace(cluster_of(item), std::make_unique<veb_tree_node_small>(id_of(item)));
                }
            }

            std::optional<T> predicate(T x) const override {
                const auto c = cluster_of(x);
                const auto i = id_of(x);

                if(x <= this->minimum) return {};
                else if(auto it = clusters.find(c); it!=clusters.end() && i>it->second->minimum) {
                    return combine(c, it->second->predicate(i).value()); 
                }
                else {
                    const T bigger_cluster = summary->predicate(c).value();
                    return combine(bigger_cluster, clusters.at(bigger_cluster)->maximum);
                }
            }

            void insert(T x) override {
                std::cout << "inserting " << std::bitset<range>(x) << " into large node of range " << num_bits << " min max: " << std::bitset<range>(this->minimum) << ',' << std::bitset<range>(this->maximum) << std::endl;
                if(x < this->minimum) {
                    this->minimum = x;
                }
                else if(x > this->maximum) {
                    this->maximum = x;
                }

                const auto c = cluster_of(x);
                const auto i = id_of(x);

                if(auto it = clusters.find(c); it == clusters.end()) {
                    std::cout << "not seen " << std::bitset<range>(x) << " in large node of range " << num_bits << std::endl;
                    summary->insert(c);
                    clusters.emplace(
                            c,
                            std::move(
                                num_bits > 16 ?
                                static_cast<std::unique_ptr<veb_tree_node>&&>(
                                    std::make_unique<veb_tree_node_large>(num_bits/2, i)
                                    ) :
                                static_cast<std::unique_ptr<veb_tree_node>&&>(
                                    std::make_unique<veb_tree_node_small>(i)
                                    )
                                )
                            );
                }
                else {
                    clusters.at(c)->insert(i);
                }
            }

            bool remove(T x) override {
                const auto c = cluster_of(x);
                const auto i = id_of(x);

                if(clusters.find(c) == clusters.end()) return false;
                else if(this->minimum == this->maximum) {
                    return true;
                }

                if(clusters.at(c)->remove(i)) {
                    clusters.erase(c);
                    summary->remove(c);
                }

                if(x == this->minimum) {
                    this->minimum = clusters.at(summary->minimum)->minimum;
                }
                else if(x == this->maximum) {
                    this->maximum = clusters.at(summary->maximum)->maximum;
                }

                return false;
            }

            ~veb_tree_node_large() {}

            private:
            T cluster_of(T item) const { return item >> num_bits/2; }
            T id_of(T item) const { return item & half_mask; }
            T combine(T c, T i) const { return c << num_bits/2 | i; }
        } the_tree;

        veb_tree(T x):
            the_tree{range, x}
        {}
};

#endif
