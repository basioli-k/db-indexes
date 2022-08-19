#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "io_handler.h"
#include "common.h"

class b_tree_node {
    static std::string path;
    int _deg;

    int32_t _nid;
    bool _is_leaf;

    int _count = 0; // number of keys in node
    std::vector<int32_t> _vals;
    
    int32_t _parent_id = -1; // if parent_id == -1 there is no parent, meaning this is the root
    std::vector<int32_t> _ptrs;

    // file name is _nid + ".btree"

    // first 4 bytes represent value of _count
    // next 4 bytes reprezent _is_leaf
    // then there are _count elements that go to _vals
    // then there are _count + 1 elements that go to _ptrs (in case the node is a leaf, the (_count+1)st element is pointer to next leaf)
    io_handler _node_f;
    
public:
    b_tree_node(int degree, int32_t nid, int32_t parent = -1) :
        _deg(degree), _nid(nid), _vals(degree), _parent_id(parent), _ptrs(degree + 1, -1), 
        _node_f(path + maybe_backslash(path) + std::to_string(_nid) + BTREE_SUFF)
    {
        // this is expected to be called only when the file was already created
        std::vector<int32_t> buff;
        _node_f.read(buff, BLOCK_SIZE / 4);
        _count = buff[0];
        _is_leaf = (buff[1] != 0);
        
        for ( size_t i = 0; i < _count; ++i ) {
            _vals[i] = buff[2 + i];
            _ptrs[i] = buff[2 + _count + i];
        }

        _ptrs[_is_leaf ? degree : _count] = buff[2 + 2 * _count];
    }

    b_tree_node(int degree, int32_t nid, bool is_leaf, int32_t parent = -1) : 
        _deg(degree), _nid(nid), _is_leaf(is_leaf), _vals(degree), _parent_id(parent), _ptrs(degree + 1, -1),
        _node_f(path + maybe_backslash(path) + std::to_string(_nid) + BTREE_SUFF)
    {
        // this constructor only creates and opens the file, we specify wheter the node is a leaf here
        // TODO maybe sanity check that file didn't already exist
        update_node();
    }
    friend class b_tree;

private:
    void update_node() {
        std::vector<int32_t> buff;

        buff.push_back(_count);
        buff.push_back(int32_t(_is_leaf));
        std::copy(_vals.begin(), _vals.begin() + _count, std::back_inserter(buff));
        std::copy(_ptrs.begin(), _ptrs.begin() + _count, std::back_inserter(buff));

        if (_count) 
            buff.push_back(_ptrs[_is_leaf ? _deg : _count]);

        _node_f.seekg(0);
        _node_f.write(buff);
    }

    bool is_empty() {
        return _count == 0;
    }

    void traverse() {
        std::cout << "NODE\n";
        for(int i = 0; i < _count ; ++i) 
            std::cout << _vals[i] << " ";
        std::cout << "\n";
        
        if(!_is_leaf) {
            for(int i = 0; i < _count + 1; ++i) {
                b_tree_node child(_deg, _ptrs[i], _nid);
                child.traverse();
            }
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
    int _deg;
    std::shared_ptr<b_tree_node> _root;
    // contains metadata, first four bytes represent the id of the root
    // next four bytes are the biggest node id used
    class metadata {
        std::unique_ptr<io_handler> _file;
        int32_t _root_id = 0;
        int32_t _max_nid = 0;
        const std::string _path;
    public:
        metadata(const std::string& path) : _path(path), _file(nullptr) {}

        void open_md() {
            _file = std::make_unique<io_handler>(_path);
        }
        
        void load() {
            if (!_file)
                open_md();
            std::vector<int32_t> buff;
            _file->read(buff, 2);
            _root_id = buff[0];
            _max_nid = buff[1];
        }

        void update_md(int32_t rid, int32_t max_nid) {
            if (!_file)
                open_md();
            _root_id = rid;
            _max_nid = max_nid;
            std::vector<int32_t> buff{_root_id, _max_nid};
            _file->write(buff);
        }

        int32_t root_id() { return _root_id; }
        int32_t max_nid() { return _max_nid; }
    };
    
    metadata _meta;
public:
    b_tree(int degree) : _deg(degree), _meta(b_tree_node::path + maybe_backslash(b_tree_node::path) + META_SUFF)   // TODO should degree be written in metadata?
    {
        auto md_path = b_tree_node::path + maybe_backslash(b_tree_node::path) + META_SUFF;

        if (file_exists(md_path)) { // root already exists
            _meta.load();
            _root = std::make_unique<b_tree_node>(_deg, _meta.root_id(), -1);
        }
        else { // root doesn't exist create it
            _meta.open_md();
            _root = std::make_unique<b_tree_node>(_deg, 0, true, -1);   // 0 is default root id
            _meta.update_md(0, 0);
        }
    }

    void traverse() {
        if(_root->is_empty()) return;
        _root->traverse();
    }
   
    void insert(int32_t val, int32_t offset) {
        if (_root->is_empty()) {
            _root->_vals[0] = val;
            _root->_ptrs[0] = offset;
            _root->_count = 1;
            _root->update_node();
            return;
        }

        auto cursor = _root;
        
        while (!cursor->_is_leaf) {
            for(int i = 0 ; i < cursor->_count; ++i) {
                if (val < cursor->_vals[i]) {
                    cursor = std::make_shared<b_tree_node>(_deg, cursor->_ptrs[i], cursor->_nid);
                    break;
                }
                if (i == cursor->_count - 1) {
                    cursor = std::make_shared<b_tree_node>(_deg, cursor->_ptrs[i + 1], cursor->_nid);
                    break;
                }
            }
        }

        insert_to_leaf(val, offset, cursor);
    }

private:
    void insert_to_leaf(int32_t val, int32_t offset, std::shared_ptr<b_tree_node> cursor) {
        int ins_ind = cursor->binary_search(val);
        assert(ins_ind == cursor->_vals.size() || cursor->_vals[ins_ind] != val); // duplicates aren't allowed
        cursor->_vals.insert(cursor->_vals.begin() + ins_ind, val);
        cursor->_ptrs.insert(cursor->_ptrs.begin() + ins_ind, offset);
        ++cursor->_count;

        // there was room in the leaf
        if (cursor->_count <= _deg) {
            cursor->_vals.resize(_deg);
            cursor->_ptrs.resize(_deg + 1);
            cursor->update_node();
            return;
        }

        _meta.update_md(_meta.root_id(), _meta.max_nid() + 1);
        auto new_leaf = std::make_shared<b_tree_node>(_deg, _meta.max_nid(), true, cursor->_parent_id);

        cursor->_count = (_deg+1) / 2 + (_deg % 2 ? 0 : 1);
        new_leaf->_count = _deg + 1 - cursor->_count;
        
        // copy values and ptrs to new leaf
        for(int i = cursor->_count ; i < cursor->_vals.size(); ++i) {
            new_leaf->_vals[i - cursor->_count] = cursor->_vals[i];
            new_leaf->_ptrs[i - cursor->_count] = cursor->_ptrs[i];
        }

        // update the last pointer in each children list
        new_leaf->_ptrs[_deg] = cursor->_ptrs[_deg + 1];    // real last pointer is at _deg + 1 because we inserted a ptr
        cursor->_ptrs[_deg] = new_leaf->_nid; // the last pointer always shows to the adjacent leaf

        // resize the vector so we don't get a memory leak and so the code up there is valid
        cursor->_vals.resize(_deg);
        cursor->_ptrs.resize(_deg + 1);
        cursor->update_node();
        new_leaf->update_node();

        // edge case, we inserted from the root
        if (cursor == _root) {
            _meta.update_md(_meta.max_nid() + 1, _meta.max_nid() + 1);
            auto new_root = std::make_shared<b_tree_node>(_deg, _meta.max_nid(), false, -1);

            cursor->_parent_id = new_root->_nid;
            new_leaf->_parent_id = new_root->_nid;
            new_root->_vals[0] = new_leaf->_vals[0];
            new_root->_ptrs[0] = cursor->_nid;
            new_root->_ptrs[1] = new_leaf->_nid;
            new_root->_count = 1;
            new_root->update_node();
            _root = new_root;
            return;
        }

        // // because of spliting propagate insertion to higher nodes
        // insert_internal(new_leaf->_vals[0], new_leaf);
    }

// TODO metadata isn't writing to disk for some reason, debug that, maybe because its a pointer
//// TODO kad krenes dalje kontroliraj metadata i kontroliraj ptrs, iako ptrs u internal ne bi trebalo raditi probleme

    // void insert_internal(int val, b_tree_node* child) {
    //     b_tree_node* parent = child->_parent;

    //     int ins_ind = parent->binary_search(val);
    //     assert(ins_ind == parent->_vals.size() || parent->_vals[ins_ind] != val); // duplicates aren't allowed
    //     parent->_vals.insert(parent->_vals.begin() + ins_ind, val);
    //     parent->_children.insert(parent->_children.begin() + ins_ind + 1, child); // values at child are greater than val
    //     ++parent->_count;

    //     // there was room in the internal node
    //     if (parent->_count <= _deg) {
    //         parent->_vals.resize(_deg);
    //         parent->_children.resize(_deg + 1);
    //         return;
    //     }

    //     b_tree_node* new_internal = new b_tree_node(_deg, false, parent->_parent);

    //     parent->_count = (_deg + 1) / 2 + (_deg % 2 ? 0 : 1);
    //     new_internal->_count = _deg - parent->_count;

    //     // copy values to new internal
    //     for (int i = 0, j = parent->_count + 1; i < new_internal->_count; i++, j++)
    //         new_internal->_vals[i] = parent->_vals[j];
        
    //     // copy pointers to new internal
    //     for (int i = 0, j = parent->_count + 1; i < new_internal->_count + 1; i++, j++) { 
    //         new_internal->_children[i] = parent->_children[j];
    //         new_internal->_children[i]->_parent = new_internal;
    //     }

    //     parent->_vals.resize(_deg);
    //     parent->_children.resize(_deg + 1);

    //     if (parent == _root) {
    //         b_tree_node* new_root = new b_tree_node(_deg, false, -1);
    //         parent->_parent = new_root;
    //         new_internal->_parent = new_root;
    //         new_root->_vals[0] = parent->_vals[parent->_count];
    //         new_root->_children[0] = parent;
    //         new_root->_children[1] = new_internal;
    //         new_root->_count = 1;
    //         _root = new_root;
    //         return;
    //     }

    //     // recurse
    //     insert_internal(parent->_vals[parent->_count], new_internal);
    // }

    
};