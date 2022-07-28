#pragma once
#include <exception>
#include <type_traits>
#include "row.h"
#include <set>
#include <unordered_map>
#include "custom_types.h"

enum class op {
    gt,
    lt,
    eq,
    land,
    lor
};

static std::string op_to_str(op operation) {
    switch (operation) {
        case op::gt:
            return ">";
        case op::lt:
            return "<";
        case op::eq:
            return "==";
        case op::land:
            return "AND";
        case op::lor:
            return "OR";
        default:
            throw std::exception("Operation type doesn't exit.\n");
    }
}

class filter;
using filter_ptr = std::unique_ptr<filter>;
class filter {
    op _op;
    std::vector<filter_ptr> _children;
    db_val _val;
    int _dim;
public:
    filter(op o, std::vector<filter_ptr> children, db_val val = {}, int dim = -1) : _op(o), _children(std::move(children)), 
    _val(val), _dim(dim)
    {
        if (!_children.size() && ((o == op::land || o == op::lor) || dim == -1)) 
            throw std::exception("Leaf nodes can't have and/or operator and have to define a dimension.");
        if ( _children.size() && (o == op::gt || o == op::lt || o == op::eq)) 
            throw std::exception("Comparators can't have any children.");
    }

    bool apply(std::unordered_map<int, const db_val>& dim_vals) {
        if (!_children.size()) {    // assume everything is satisfied if you get one value
            return cmp(dim_vals[_dim]);   
        }
            
        std::vector<bool> applied;
        applied.reserve(_children.size());
        
        for(size_t i = 0 ; i < _children.size(); ++i)
            applied.push_back(_children[i]->apply(dim_vals));
        
        return logical_op(applied);
    }

    bool apply(row& row) {
        if (!_children.size())
            return cmp(row.get_val(_dim));

        std::vector<bool> applied;
        applied.reserve(_children.size());
        
        for(size_t i = 0 ; i < _children.size(); ++i)
            applied.push_back(_children[i]->apply(row));
        
        return logical_op(applied);
    }

    std::string get_filter_text(schema& schema) {
        if (!_children.size())
            return schema.get_column(_dim).name + " " + op_to_str(_op) + " " + _val.to_string();
        
        std::string text = "(";
        for (size_t i = 0 ; i < _children.size(); ++i) {
            text += _children[i]->get_filter_text(schema);
            if (i == _children.size() - 1) text += ")";
            else text += " " + op_to_str(_op) + " ";
        }

        return text;
    }

    void get_filter_dims(std::set<int>& dims) {
        if (!_children.size())
            dims.insert(_dim);
        
        for (size_t i = 0; i < _children.size(); ++i)
            _children[i]->get_filter_dims(dims);
    }

private:
    bool cmp(const db_val& val) {
        if (_op == op::eq) return val == _val;
        else if (_op == op::lt) return val < _val;
        else if (_op == op::gt) return val > _val;
        else throw std::exception("Invalid use of cmp.\n");
    }

    bool logical_op(const std::vector<bool>& bool_vals) {
        if (_op == op::land) {
            bool result = true;
            for(auto val : bool_vals) result = result && val;
            return result;
        } 
        else if (_op == op::lor) {
            bool result = false;
            for(auto val : bool_vals) result = result || val;
            return result;
        }
        else throw std::exception("Invalid use of logical op.\n");
    }
};