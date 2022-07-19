#pragma once
#include <exception>
#include <type_traits>
#include "row.h"

enum class op {
    ge,
    le,
    eq,
    land,
    lor
};

class filter_base {
protected:
    using base_ptr = std::unique_ptr<filter_base>;
    op _op;
    base_ptr _left, _right;
public:
    filter_base(op o, base_ptr l, base_ptr r) : _op(o), _left(std::move(l)), 
        _right(std::move(r)) {
            if (!_left && !_right && (o == op::land || o == op::lor)) throw std::exception("Leaf nodes can't have and/or operator.");
            if ( (_left || _right) && (o == op::ge || o == op::le || o == op::eq)) throw std::exception("Comparators can't have any children.");
    }

    virtual bool apply(row& row, int dim) = 0;
};

template <typename T>
class filter : public filter_base {
    T _val;
public:
    filter(T val, op o, base_ptr l, base_ptr r) : filter_base(o, std::move(l), std::move(r)),
        _val(val) {}

    bool apply(row& row, int dim) {
        if (!_left && !_right) {
            auto val = row.get_val(dim);
            return cmp<decltype(val)>(row.get_val(dim));
        }
        else if (!_left)
            return _right->apply(row, dim);
        else if (!_right)
            return _left->apply(row, dim);
        
        return logical_op(_left->apply(row, dim), _right->apply(row, dim));
    }
private:
    template <typename S>
    typename std::enable_if<std::is_same_v<T, S>, bool>::type cmp(S val) {
        if (_op == op::eq) return _val == val;
        else if (_op == op::le) return _val < val;
        else if (_op == op::ge) return _val > val;
        else throw std::exception("Invalid use of cmp.\n");
    }

    bool logical_op(bool b1, bool b2) {
        if (_op == op::land) return b1 && b2;
        else if (_op == op::lor) return b1 || b2;
        else throw std::exception("Invalid use of logical op.\n");
    }
};