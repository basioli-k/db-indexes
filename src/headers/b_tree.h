#pragma once
#include <filesystem>
#include <iostream>
#include <vector>
#include <string>
#include "io_handler.h"
#include "query.h"
#include "hor_table.h"
#include "common.h"

class b_tree_node {
    static std::string path;
    int _deg;

    int32_t _nid;
    bool _is_leaf;

    int _count = 0; // number of keys in node
    std::vector<int32_t> _vals;

    std::vector<int32_t> _ptrs;

    // file name is _nid + ".btree"

    // first 4 bytes represent value of _count
    // next 4 bytes reprezent _is_leaf
    // next 4 bytes reprezent _parent
    // then there are _count elements that go to _vals
    // then there are _count + 1 elements that go to _ptrs (in case the node is a leaf, the (_count+1)st element is pointer to next leaf)
    io_handler _node_f;
    
public:
    b_tree_node(int degree, int32_t nid) :
        _deg(degree), _nid(nid), _vals(degree), _ptrs(degree + 1, -1), 
        _node_f(path + maybe_backslash(path) + std::to_string(_nid) + BTREE_SUFF)
    {
        // this is expected to be called only when the file was already created
        std::vector<int32_t> buff;
        _node_f.read(buff, BLOCK_SIZE / 4);
        _count = buff[0];
        _is_leaf = (buff[1] != 0);
    
        for ( size_t i = 0; i < _count; ++i ) {
            _vals[i] = buff[TREE_NODE_HDR_LEN + i];
            _ptrs[i] = buff[TREE_NODE_HDR_LEN + _count + i];
        }

        _ptrs[_is_leaf ? degree : _count] = buff[TREE_NODE_HDR_LEN + 2 * _count];
    }

    b_tree_node(int degree, int32_t nid, bool is_leaf) : 
        _deg(degree), _nid(nid), _is_leaf(is_leaf), _vals(degree), _ptrs(degree + 1, -1),
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
        if (_is_leaf) std::cout << "LEAF ";
        std::cout << "NODE " << _nid << "\n";
        
        // TODO temporary
        if(!_is_leaf) {
            std::cout << "Children:\n";
            for(int i = 0; i < _count + 1; ++i)
                std::cout << _ptrs[i] << " ";
            std::cout << "\n";
        }
        
        std::cout << "Vals:\n";
        for(int i = 0; i < _count ; ++i) 
            std::cout << _vals[i] << " ";
        std::cout << "\n";
        
        if(!_is_leaf) {
            for(int i = 0; i < _count + 1; ++i) {
                b_tree_node child(_deg, _ptrs[i]);
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
    int _no_of_reads = 0; // variable for counting reads

    class metadata {
        std::shared_ptr<io_handler> _file;
        int32_t _root_id = 0;
        int32_t _max_nid = 0;
        const std::string _path;
    public:
        metadata(const std::string& path) : _path(path), _file(nullptr) {}
        
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
            _file->seekg(0);
            _file->write(buff);
        }

        int32_t root_id() { return _root_id; }
        int32_t max_nid() { return _max_nid; }

    private:
        void open_md() {
            _file = std::make_unique<io_handler>(_path);
        }
    };
    
    metadata _meta;
public:
    b_tree(int degree) : _deg(degree), _meta(b_tree_node::path + maybe_backslash(b_tree_node::path) + META_SUFF)   // TODO should degree be written in metadata?
    {
        auto md_path = b_tree_node::path + maybe_backslash(b_tree_node::path) + META_SUFF;

        if (file_exists(md_path)) { // root already exists
            _meta.load();
            _root = std::make_shared<b_tree_node>(_deg, _meta.root_id());
        }
        else { // root doesn't exist create it
            create_ind_folder();
            _root = std::make_shared<b_tree_node>(_deg, 0, true);   // 0 is default root id
            _meta.update_md(0, 0);
        }
    }

    void traverse() {
        if(_root->is_empty()) return;
        _root->traverse();
    }

    int no_of_reads() { return _no_of_reads; }

    void reset_reads() { _no_of_reads = 0; }
   
    int32_t search(int32_t val) {
        auto cursor = _root;

        while (!cursor->_is_leaf) {
            for(int i = 0 ; i < cursor->_count; ++i) {
                if (val < cursor->_vals[i]) {
                    cursor = std::make_shared<b_tree_node>(_deg, cursor->_ptrs[i]);
                    _no_of_reads++;
                    break;
                }
                if (i == cursor->_count - 1) {
                    cursor = std::make_shared<b_tree_node>(_deg, cursor->_ptrs[i + 1]);
                    _no_of_reads++;
                    break;
                }
            }
        }
        auto index = cursor->binary_search(val);
        if (val == cursor->_vals[index]) 
            return cursor->_ptrs[index];
        return -1;
    }

    // includes low and high
    std::vector<int32_t> search_range(int32_t low, int32_t high) {
        std::vector<int32_t> results;
        auto cursor = _root;

        while (!cursor->_is_leaf) {
            for(int i = 0 ; i < cursor->_count; ++i) {
                if (low < cursor->_vals[i]) {
                    cursor = std::make_shared<b_tree_node>(_deg, cursor->_ptrs[i]);
                    _no_of_reads++;
                    break;
                }
                if (i == cursor->_count - 1) {
                    cursor = std::make_shared<b_tree_node>(_deg, cursor->_ptrs[i + 1]);
                    _no_of_reads++;
                    break;
                }
            }
        }
        results.shrink_to_fit();

        return results;
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
                    cursor = std::make_shared<b_tree_node>(_deg, cursor->_ptrs[i]);
                    break;
                }
                if (i == cursor->_count - 1) {
                    cursor = std::make_shared<b_tree_node>(_deg, cursor->_ptrs[i + 1]);
                    break;
                }
            }
        }

        insert_to_leaf(val, offset, cursor);
    }

private:
    void create_ind_folder() {
        std::filesystem::create_directories(b_tree_node::path);
    }

    void insert_to_leaf(int32_t val, int32_t offset, std::shared_ptr<b_tree_node> cursor) {
        int ins_ind = cursor->binary_search(val);
        assert(ins_ind == cursor->_vals.size() || cursor->_vals[ins_ind] != val); // duplicates aren't allowed
        cursor->_vals.insert(cursor->_vals.begin() + ins_ind, val);
        cursor->_ptrs.insert(cursor->_ptrs.begin() + ins_ind, offset);
        ++cursor->_count;

        // there was room in the leaf
        if (cursor->_count <= _deg) {
            cursor->_vals.resize(_deg);
            cursor->_ptrs[_deg] = cursor->_ptrs[cursor->_ptrs.size() - 1];
            cursor->_ptrs.resize(_deg + 1);
            cursor->update_node();
            return;
        }

        _meta.update_md(_meta.root_id(), _meta.max_nid() + 1);
     
        auto new_leaf = std::make_shared<b_tree_node>(_deg, _meta.max_nid(), true);

        cursor->_count = (_deg+1) / 2 + (_deg % 2 ? 0 : 1);
        new_leaf->_count = _deg + 1 - cursor->_count;
        
        // copy values and ptrs to new leaf
        for(int i = cursor->_count ; i < cursor->_vals.size(); ++i) {
            new_leaf->_vals[i - cursor->_count] = cursor->_vals[i];
            new_leaf->_ptrs[i - cursor->_count] = cursor->_ptrs[i];
        }

        // update the last pointer in each children list
        new_leaf->_ptrs[_deg] = cursor->_ptrs[_deg+1];    // real last pointer is at _deg + 1 because we inserted a ptr
        cursor->_ptrs[_deg] = new_leaf->_nid; // the last pointer always shows to the adjacent leaf

        // resize the vector so we don't get a memory leak and so the code up there is valid
        cursor->_vals.resize(_deg);
        cursor->_ptrs.resize(_deg + 1);
        cursor->update_node();
        new_leaf->update_node();

        // edge case, we inserted to the root
        if (cursor->_nid == _root->_nid) {
            _meta.update_md(_meta.max_nid() + 1, _meta.max_nid() + 1);
            auto new_root = std::make_shared<b_tree_node>(_deg, _meta.max_nid(), false);

            new_root->_vals[0] = new_leaf->_vals[0];
            new_root->_ptrs[0] = cursor->_nid;
            new_root->_ptrs[1] = new_leaf->_nid;
            new_root->_count = 1;
            new_root->update_node();
            _root = new_root;
            return;
        }

        // because of spliting propagate insertion to higher nodes
        
        int32_t par_id = find_parent(cursor);
        auto parent = par_id == _root->_nid ? _root : std::make_shared<b_tree_node>(_deg, par_id);
        insert_internal(new_leaf->_vals[0], new_leaf, parent);
    }

    void insert_internal(int32_t val, std::shared_ptr<b_tree_node> cursor, std::shared_ptr<b_tree_node> parent) {
        int ins_ind = parent->binary_search(val);
        assert(ins_ind == parent->_vals.size() || parent->_vals[ins_ind] != val); // duplicates aren't allowed
        parent->_vals.insert(parent->_vals.begin() + ins_ind, val);

        parent->_ptrs.insert(parent->_ptrs.begin() + ins_ind + 1, cursor->_nid); // for +1 check example
        ++parent->_count;

        // there was room in the internal node
        if (parent->_count <= _deg) {
            parent->_vals.resize(_deg);
            parent->_ptrs.resize(_deg + 1);
            parent->update_node();
            return;
        }

        _meta.update_md(_meta.root_id(), _meta.max_nid() + 1);
        auto new_internal = std::make_shared<b_tree_node>(_deg, _meta.max_nid(), false);

        parent->_count = _deg / 2 + (_deg % 2);
        new_internal->_count = _deg - parent->_count;

        // copy values to new internal
        // j = parent->_count + 1 (+1 because the middle one is going to be used for spliting between the two nodes)
        for (int i = 0, j = parent->_count + 1; i < new_internal->_count; i++, j++) 
            new_internal->_vals[i] = parent->_vals[j];
        
        // copy pointers to new internal
        for (int i = 0, j = parent->_count + 1; i < new_internal->_count + 1; i++, j++)
            new_internal->_ptrs[i] = parent->_ptrs[j];

        parent->_vals.resize(_deg);
        parent->_ptrs.resize(_deg + 1);

        parent->update_node();
        new_internal->update_node();

        if (parent->_nid == _root->_nid) {
            _meta.update_md(_meta.max_nid() + 1, _meta.max_nid() + 1);
            auto new_root = std::make_shared<b_tree_node>(_deg, _meta.max_nid(), false);

            new_root->_vals[0] = parent->_vals[parent->_count];
            new_root->_ptrs[0] = parent->_nid;
            new_root->_ptrs[1] = new_internal->_nid;
            new_root->_count = 1;
            _root = new_root;

            new_root->update_node();
            return;
        }

        // recurse
        int32_t int_par_id = find_parent(parent);
        auto internal_parent = int_par_id == _root->_nid ? _root : std::make_shared<b_tree_node>(_deg, int_par_id);
        insert_internal(parent->_vals[parent->_count], new_internal, internal_parent);
    }

    int32_t find_parent(std::shared_ptr<b_tree_node> child) {
        auto cursor = _root;
        auto val = child->_vals[0];

        while (std::find(cursor->_ptrs.begin(), cursor->_ptrs.begin() + cursor->_count + 1, child->_nid) == (cursor->_ptrs.begin() + cursor->_count + 1) 
            && !cursor->_is_leaf) 
        {
            for(int i = 0 ; i < cursor->_count; ++i) {
                if (val < cursor->_vals[i]) {
                    cursor = std::make_shared<b_tree_node>(_deg, cursor->_ptrs[i]);
                    break;
                }
                if (i == cursor->_count - 1) {
                    cursor = std::make_shared<b_tree_node>(_deg, cursor->_ptrs[i + 1]);
                    break;
                }
            }
        }

        return cursor->_is_leaf ? -1 : cursor->_nid;
    }
};

// creates btree on col col_index
static b_tree create_b_tree(hor_table& table, size_t col_index) {
    b_tree btree(NODE_PARAM);
    query q(nullptr, query_type::star, 0); // select all

    auto res = table.execute_query(q);

    for(size_t i = 0 ; i < res.size(); ++i) {
        auto val = res.rows()[i].get_val(col_index);
        btree.insert(int32_t(val), i);  // TODO works for int32_t
    }

    return btree;
}