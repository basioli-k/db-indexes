#pragma once
#include <iostream>
#include <vector>

inline int node_id = 0; // TODO remove this, or use it for some reason

class b_tree_node {
    int nid;
    int _count = 0; // number of keys in node
    int _deg;
    bool _is_leaf;
    b_tree_node* _parent;
    std::vector<int> _vals;
    std::vector<b_tree_node*> _children; // testing purposes
    // std::vector<int> _offsets;
public:
    b_tree_node(int degree, bool is_leaf, b_tree_node* parent) : 
            _is_leaf(is_leaf),
            _parent(parent),
            _deg(degree),
            _vals(degree), // ako je _offset[i], odnosno _children[i] nevazec ovaj _vals[i] je zanemaren
            // _offsets(degree + 1, -1)
            _children(degree+1, nullptr) { nid = node_id++;}
    friend class b_tree;

private:
    void traverse(int level = 0) {
        if (_is_leaf) std::cout << "LEAF ";
        std::cout << "NODE " << level << "\n";
        for(int i = 0; i < _count ; ++i) 
            std::cout << _vals[i] << " ";
        
        std::cout << "\n";
        
        if(!_is_leaf) {
            for(int i = 0; i < _count + 1; ++i)
                _children[i]->traverse(level+1);
        }
    }

    // TODO delete this, POC function to check if leaves are connected
    void traverse_leaves(int level = 0) {
        if (_is_leaf) {
            for(int i = 0; i < _count ; ++i)
                std::cout << _vals[i] << " ";
            std::cout << "\n";
        }
        
        if (!_is_leaf) {
            _children[0]->traverse_leaves(level+1);
        }
        else if (_children[_deg]){
            _children[_deg]->traverse_leaves(level+1);
        }
    }

    // returns index where val should go to
    int binary_search(int val) {
        int low = 0, high = _count - 1;
        int mid;

        while ( low <= high) {
            mid = low + (high - low) / 2;

            if ( val == _vals[mid])
                return mid;
            else if (_vals[mid] < val)
                low = mid + 1;
            else
                high = mid -1;
        }

        return high + 1;
    }
};


class b_tree {
    b_tree_node* _root;
    int _deg;
public:
    b_tree(int degree) : _deg(degree) {
        _root = nullptr;
    }

    void traverse() {
        if(!_root) return;
        _root->traverse();
    }
    void insert(int val) {
        if (!_root) {
            _root = new b_tree_node(_deg, true, nullptr);
            _root->_vals[0] = val;
            _root->_count = 1;
            return;
        }

        b_tree_node* cursor = _root;
        
        while (!cursor->_is_leaf) {
            for(int i = 0 ; i < cursor->_count; ++i) {
                if (val < cursor->_vals[i]) {
                    cursor = cursor->_children[i];
                    break;
                }
                if (i == cursor->_count - 1) {
                    cursor = cursor->_children[i + 1];
                    break;
                }
            }
        }

        insert_to_leaf(val, cursor);
    }

private:
    void insert_to_leaf(int val, b_tree_node* cursor) {
        int ins_ind = cursor->binary_search(val);
        assert(ins_ind == cursor->_vals.size() || cursor->_vals[ins_ind] != val); // duplicates aren't allowed
        cursor->_vals.insert(cursor->_vals.begin() + ins_ind, val);
        ++cursor->_count;
        // there was room in the leaf
        if (cursor->_count <= _deg) {
            cursor->_vals.resize(_deg);
            return;
        }

        b_tree_node* new_leaf = new b_tree_node(_deg, true, cursor->_parent);

        cursor->_count = (_deg+1) / 2 + (_deg % 2 ? 0 : 1);
        new_leaf->_count = _deg + 1 - cursor->_count;
        
        // update the last pointer in each children list
        new_leaf->_children[_deg] = cursor->_children[_deg];
        cursor->_children[_deg] = new_leaf; // the last pointer always shows to the adjacent leaf
        
        // copy values to new leaf
        for(int i = cursor->_count ; i < cursor->_vals.size(); ++i)
            new_leaf->_vals[i - cursor->_count] = cursor->_vals[i];

        // resize the vector so we don't get a memory leak and so the code up there is valid
        cursor->_vals.resize(_deg);

        // edge case, we inserted from the root
        if (cursor == _root) {
            b_tree_node* new_root = new b_tree_node(_deg, false, nullptr);
            cursor->_parent = new_root;
            new_leaf->_parent = new_root;
            new_root->_vals[0] = new_leaf->_vals[0];
            new_root->_children[0] = cursor;
            new_root->_children[1] = new_leaf;
            new_root->_count = 1;
            _root = new_root;
            return;
        }

        // because of spliting propagate insertion to higher nodes
        insert_internal(new_leaf->_vals[0], new_leaf);
    }

    void insert_internal(int val, b_tree_node* child) {
        b_tree_node* parent = child->_parent;

        int ins_ind = parent->binary_search(val);
        assert(ins_ind == parent->_vals.size() || parent->_vals[ins_ind] != val); // duplicates aren't allowed
        parent->_vals.insert(parent->_vals.begin() + ins_ind, val);
        parent->_children.insert(parent->_children.begin() + ins_ind + 1, child); // values at child are greater than val
        ++parent->_count;

        // there was room in the internal node
        if (parent->_count <= _deg) {
            parent->_vals.resize(_deg);
            parent->_children.resize(_deg + 1);
            return;
        }

        b_tree_node* new_internal = new b_tree_node(_deg, false, parent->_parent);

        parent->_count = (_deg + 1) / 2 + (_deg % 2 ? 0 : 1);
        new_internal->_count = _deg - parent->_count;

        // copy values to new internal
        for (int i = 0, j = parent->_count + 1; i < new_internal->_count; i++, j++)
            new_internal->_vals[i] = parent->_vals[j];
        
        // copy pointers to new internal
        for (int i = 0, j = parent->_count + 1; i < new_internal->_count + 1; i++, j++) { 
            new_internal->_children[i] = parent->_children[j];
            new_internal->_children[i]->_parent = new_internal;
        }

        parent->_vals.resize(_deg);
        parent->_children.resize(_deg + 1);

        if (parent == _root) {
            b_tree_node* new_root = new b_tree_node(_deg, false, nullptr);
            parent->_parent = new_root;
            new_internal->_parent = new_root;
            new_root->_vals[0] = parent->_vals[parent->_count];
            new_root->_children[0] = parent;
            new_root->_children[1] = new_internal;
            new_root->_count = 1;
            _root = new_root;
            return;
        }

        // recurse
        insert_internal(parent->_vals[parent->_count], new_internal);
    }

    
};