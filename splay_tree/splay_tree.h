#ifndef SPLAY_TREE_H
#define SPLAY_TREE_H

#include <iostream>
#include <functional>
#include <memory>


template<typename K, typename V, typename Comparator=std::less<K>> class splay_tree {
    struct splay_tree_node {
        K key;
        V val;
        splay_tree_node* parent, *left_child, *right_child;

        template<typename X=K, typename Y=V> splay_tree_node(
                X &&key,
                Y &&val,
                splay_tree_node *parent,
                splay_tree_node *left_child,
                splay_tree_node *right_child
                ):
            key{std::forward<X>(key)},
            val{std::forward<Y>(val)},
            parent{parent},
            left_child{left_child},
            right_child{right_child}
        {}

        void print(size_t depth=0) const {
            for(size_t k=0; k<depth; ++k) std::cout << " ";
            std::cout << '(' << key << ", " << val << ')' << std::endl;
            
            if(left_child) left_child->print(depth + 1);
            else std::cout << std::endl;
            if(right_child) right_child->print(depth + 1);
            else std::cout << std::endl;
        } 

        ~splay_tree_node() {
            delete left_child;
            delete right_child;
        }
    };


    splay_tree_node *traverse_parent(const K& key) const {
        splay_tree_node *prev = nullptr;
        splay_tree_node *current = the_tree;

        while(current) {
            prev = current;

            if(comp(key, current->key)) current = current->left_child;
            else if(key == current->key) break;
            else current = current->right_child;
        }

        return prev;

    }

    splay_tree_node *traverse(const K& key) const {
        splay_tree_node *current = traverse_parent(key);

        if(current) {
            if(comp(key, current->key)) return current->left_child;
            else if(key == current->key) return current;
            else return current->right_child;
        }
        else return nullptr;
    }

    splay_tree_node* left(splay_tree_node* child, splay_tree_node* parent) {
        splay_tree_node *child1 = child->left_child;

        child->parent = parent->parent;
        parent->parent = child;

        parent->right_child = child1;

        child->left_child = parent;

        return child;
    }

    splay_tree_node* right(splay_tree_node* child, splay_tree_node* parent) {
        splay_tree_node *child1 = child->right_child;

        child->parent = parent->parent;
        parent->parent = child;

        parent->left_child = child1;

        child->right_child = parent;

        return child;
    }

    splay_tree_node* left_left(splay_tree_node* child, splay_tree_node* parent, splay_tree_node* grandparent) {
        splay_tree_node *child1 = parent->left_child;
        splay_tree_node *child2 = child->left_child;

        child->parent = grandparent->parent;
        parent->parent = child;
        grandparent->parent = parent;

        grandparent->right_child = child1;

        parent->left_child = grandparent;
        parent->right_child = child2;

        child->left_child = parent;

        return child;
    }

    splay_tree_node* right_right(splay_tree_node* child, splay_tree_node* parent, splay_tree_node* grandparent) {
        splay_tree_node *child1 = child->right_child;
        splay_tree_node *child2 = parent->right_child;

        child->parent = grandparent->parent;
        parent->parent = child;
        grandparent->parent = parent;

        grandparent->left_child = child2;

        parent->left_child = child1;
        parent->right_child = grandparent;

        child->right_child = parent;

        return child;
    }
    splay_tree_node* left_right(splay_tree_node* child, splay_tree_node* parent, splay_tree_node* grandparent) {
        splay_tree_node *child1 = child->left_child;
        splay_tree_node *child2 = child->right_child;

        child->parent = grandparent->parent;
        parent->parent = child;
        grandparent->parent = child;

        grandparent->left_child = child2;

        parent->right_child = child1;

        child->left_child = parent;
        child->right_child = grandparent;

        return child;
    }
    splay_tree_node* right_left(splay_tree_node* child, splay_tree_node* parent, splay_tree_node* grandparent) {
        splay_tree_node *child1 = child->left_child;
        splay_tree_node *child2 = child->right_child;

        child->parent = grandparent->parent;
        parent->parent = child;
        grandparent->parent = child;

        grandparent->right_child = child1;

        parent->left_child = child2;

        child->left_child = grandparent;
        child->right_child = parent;

        return child;
    }

    bool is_left_child (splay_tree_node *child, splay_tree_node *parent) const {
        return parent->left_child == child;
    }

    void splay(splay_tree_node * node) {
        if(!node) return;
    
        while(node->parent) {
            splay_tree_node *parent = node->parent;

            if(parent->parent) {
                splay_tree_node* grandparent = parent->parent;

                splay_tree_node *&child_ptr = grandparent->parent ?
                    (is_left_child(grandparent, grandparent->parent) ? grandparent->parent->left_child : grandparent->parent->right_child) :
                    the_tree;

                if(is_left_child(parent, grandparent)) {
                    if(is_left_child(node, parent)) child_ptr = right_right(node, parent, grandparent);
                    else child_ptr = left_right(node, parent, grandparent);
                }
                else {
                    if(is_left_child(node, parent)) child_ptr = right_left(node, parent, grandparent);
                    else child_ptr = left_left(node, parent, grandparent);
                }
            }
            else {
                the_tree = is_left_child(node, parent) ? right(node, parent) : left(node, parent);
                return;
            }
        }
    }

    void splay(const K& key) {
        splay_tree_node* node = traverse(key);
        splay(node);
    }

    Comparator comp;
    splay_tree_node *the_tree;

    public:
    splay_tree():
        comp{},
        the_tree{}
    {}

    std::optional<V&> find(const K& key) {
        if(!the_tree) return {};

        splay(key);

        if(the_tree->key == key) return the_tree->val;
        else return {};
    }

    template<typename X=K, typename Y=V> void insert(X &&key, Y &&val) {
        splay_tree_node* parent = traverse_parent(std::forward<X>(key));

        if(parent) {
            if(comp(key, parent->key)) {
                if(parent->left_child) parent->left_child->val = std::forward<Y>(val);
                else parent->left_child = new splay_tree_node{std::forward<X>(key), std::forward<Y>(val), parent, nullptr, nullptr};

                splay(parent->left_child);
            }
            else if(key == parent->key) {
                parent->val = std::forward<Y>(val);
                splay(parent);
            }
            else {
                if(parent->right_child) parent->right_child->val = std::forward<Y>(val);
                else parent->right_child = new splay_tree_node{std::forward<X>(key), std::forward<Y>(val), parent, nullptr, nullptr};

                splay(parent->right_child);
            }
        }
        else {
            the_tree = new splay_tree_node{std::forward<X>(key), std::forward<Y>(val), nullptr, nullptr, nullptr};
        }
    }

    void print() const { if(the_tree) the_tree->print(); }

    ~splay_tree() { delete the_tree; }
};

#endif
