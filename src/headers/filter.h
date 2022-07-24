#pragma once
#include <exception>
#include <type_traits>
#include "row.h"
#include "custom_types.h"

enum class op {
    gt,
    lt,
    eq,
    land,
    lor
};

class filter {
    using filter_ptr = std::unique_ptr<filter>;
    op _op;
    filter_ptr _left, _right;
    db_val _val;
    int _dim;
public:
    filter(op o, filter_ptr l, filter_ptr r, db_val val = {}, int dim = -1) : _op(o), _left(std::move(l)), 
        _right(std::move(r)), _val(val), _dim(dim)
    {
        if (!_left && !_right && ((o == op::land || o == op::lor) || dim == -1)) 
            throw std::exception("Leaf nodes can't have and/or operator and have to define a dimension.");
        if ( (_left || _right) && (o == op::gt || o == op::lt || o == op::eq)) throw std::exception("Comparators can't have any children.");
    }

    bool apply(const db_val& val, int dim) {
        if (!_left && !_right) {    // assume everything is satisfied if you get one value
            return dim == _dim ? cmp(val) : true;   
        }
            
        else if (!_left)
            return _right->apply(val, dim);
        else if (!_right)
            return _left->apply(val, dim);
        
        return logical_op(_left->apply(val, dim), _right->apply(val, dim));
    }

    bool apply(row& row) {
        if (!_left && !_right)
            return cmp(row.get_val(_dim));
        else if (!_left)
            return _right->apply(row);
        else if (!_right)
            return _left->apply(row);
        
        return logical_op(_left->apply(row), _right->apply(row));
    }
private:
    bool cmp(const db_val& val) {
        if (_op == op::eq) return val == _val;
        else if (_op == op::lt) return val < _val;
        else if (_op == op::gt) return val > _val;
        else throw std::exception("Invalid use of cmp.\n");
    }

    bool logical_op(bool b1, bool b2) {
        if (_op == op::land) return b1 && b2;
        else if (_op == op::lor) return b1 || b2;
        else throw std::exception("Invalid use of logical op.\n");
    }
};