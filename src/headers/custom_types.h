#pragma once
#include <string>
#include <variant>
#include <iostream>
// everything related to types should be in this file

using type_variant = std::variant<int32_t, uint32_t, int64_t, uint64_t, float, double>;
enum class types {
    int32_t,
    uint32_t,
    int64_t,
    uint64_t,
    float32,
    float64
};

types get_type(const std::string& str_type) {
    if (str_type == "int32_t")
        return types::int32_t;
    else if (str_type == "uint32_t")
        return types::uint32_t;
    else if (str_type == "int64_t")
        return types::int64_t;
    else if (str_type == "uint64_t")
        return types::uint64_t;
    else if (str_type == "float")
        return types::float32;
    else if (str_type == "double")
        return types::float64;
    else
        throw std::invalid_argument("Unsupported type.");
}

std::size_t get_size(types type) {
    switch(type){
        case types::int32_t:
            return sizeof(int32_t);
        case types::uint32_t:
            return sizeof(uint32_t);
        case types::int64_t:
            return sizeof(int64_t);
        case types::uint64_t:
            return sizeof(uint64_t);
        case types::float32:
            return sizeof(float);
        case types::float64:
            return sizeof(double);
        default:
            throw std::invalid_argument("Unsupported type.");
    }
}

class db_val {
    type_variant _val;
public:
    db_val() {}

    template <typename T>
    db_val(T val) : _val(val) {}

    db_val operator+(const db_val& oth) {
        assert(_val.index() == oth._val.index());

        switch (static_cast<types>(_val.index())) {
            case types::int32_t:
                return db_val(std::get<0>(_val) + std::get<0>(oth._val));
            case types::uint32_t:
                return db_val(std::get<1>(_val) + std::get<1>(oth._val));
            case types::int64_t:
                return db_val(std::get<2>(_val) + std::get<2>(oth._val));
            case types::uint64_t:
                return db_val(std::get<3>(_val) + std::get<3>(oth._val));
            case types::float32:
                return db_val(std::get<4>(_val) + std::get<4>(oth._val));
            case types::float64:
                return db_val(std::get<5>(_val) + std::get<5>(oth._val));
            default:
                throw std::invalid_argument("Unsupported type.");
        }
    }

    db_val operator-(const db_val& oth) {
        assert(_val.index() == oth._val.index());

        switch (static_cast<types>(_val.index())) {
            case types::int32_t:
                return db_val(std::get<0>(_val) - std::get<0>(oth._val));
            case types::uint32_t:
                return db_val(std::get<1>(_val) - std::get<1>(oth._val));
            case types::int64_t:
                return db_val(std::get<2>(_val) - std::get<2>(oth._val));
            case types::uint64_t:
                return db_val(std::get<3>(_val) - std::get<3>(oth._val));
            case types::float32:
                return db_val(std::get<4>(_val) - std::get<4>(oth._val));
            case types::float64:
                return db_val(std::get<5>(_val) - std::get<5>(oth._val));
            default:
                throw std::invalid_argument("Unsupported type.");
        }
    }

    db_val operator*(const db_val& oth) {
        assert(_val.index() == oth._val.index());

        switch (static_cast<types>(_val.index())) {
            case types::int32_t:
                return db_val(std::get<0>(_val) * std::get<0>(oth._val));
            case types::uint32_t:
                return db_val(std::get<1>(_val) * std::get<1>(oth._val));
            case types::int64_t:
                return db_val(std::get<2>(_val) * std::get<2>(oth._val));
            case types::uint64_t:
                return db_val(std::get<3>(_val) * std::get<3>(oth._val));
            case types::float32:
                return db_val(std::get<4>(_val) * std::get<4>(oth._val));
            case types::float64:
                return db_val(std::get<5>(_val) * std::get<5>(oth._val));
            default:
                throw std::invalid_argument("Unsupported type.");
        }
    }

    db_val operator/(const db_val& oth) {
        assert(_val.index() == oth._val.index());

        switch (static_cast<types>(_val.index())) {
            case types::int32_t:
                return db_val(std::get<0>(_val) / std::get<0>(oth._val));
            case types::uint32_t:
                return db_val(std::get<1>(_val) / std::get<1>(oth._val));
            case types::int64_t:
                return db_val(std::get<2>(_val) / std::get<2>(oth._val));
            case types::uint64_t:
                return db_val(std::get<3>(_val) / std::get<3>(oth._val));
            case types::float32:
                return db_val(std::get<4>(_val) / std::get<4>(oth._val));
            case types::float64:
                return db_val(std::get<5>(_val) / std::get<5>(oth._val));
            default:
                throw std::invalid_argument("Unsupported type.");
        }
    }

    db_val operator/(const size_t oth) {
        switch (static_cast<types>(_val.index())) {
            case types::int32_t:
                return db_val(int32_t(std::get<0>(_val) / oth));
            case types::uint32_t:
                return db_val(uint32_t(std::get<1>(_val) / oth));
            case types::int64_t:
                return db_val(int64_t(std::get<2>(_val) / oth));
            case types::uint64_t:
                return db_val(uint64_t(std::get<3>(_val) / oth));
            case types::float32:
                return db_val(float(std::get<4>(_val) / oth));
            case types::float64:
                return db_val(double(std::get<5>(_val) / oth));
            default:
                throw std::invalid_argument("Unsupported type.");
        }
    }

    bool operator<(const db_val& oth) const {
        assert(_val.index() == oth._val.index());

        switch (static_cast<types>(_val.index())) {
            case types::int32_t:
                return std::get<0>(_val) < std::get<0>(oth._val);
            case types::uint32_t:
                return std::get<1>(_val) < std::get<1>(oth._val);
            case types::int64_t:
                return std::get<2>(_val) < std::get<2>(oth._val);
            case types::uint64_t:
                return std::get<3>(_val) < std::get<3>(oth._val);
            case types::float32:
                return std::get<4>(_val) < std::get<4>(oth._val);
            case types::float64:
                return std::get<5>(_val) < std::get<5>(oth._val);
            default:
                throw std::invalid_argument("Unsupported type.");
        }
    }

    bool operator>(const db_val& oth) const {
        assert(_val.index() == oth._val.index());

        switch (static_cast<types>(_val.index())) {
            case types::int32_t:
                return std::get<0>(_val) > std::get<0>(oth._val);
            case types::uint32_t:
                return std::get<1>(_val) > std::get<1>(oth._val);
            case types::int64_t:
                return std::get<2>(_val) > std::get<2>(oth._val);
            case types::uint64_t:
                return std::get<3>(_val) > std::get<3>(oth._val);
            case types::float32:
                return std::get<4>(_val) > std::get<4>(oth._val);
            case types::float64:
                return std::get<5>(_val) > std::get<5>(oth._val);
            default:
                throw std::invalid_argument("Unsupported type.");
        }
    }

    bool operator==(const db_val& oth) const {
        assert(_val.index() == oth._val.index());

        switch (static_cast<types>(_val.index())) {
            case types::int32_t:
                return std::get<0>(_val) == std::get<0>(oth._val);
            case types::uint32_t:
                return std::get<1>(_val) == std::get<1>(oth._val);
            case types::int64_t:
                return std::get<2>(_val) == std::get<2>(oth._val);
            case types::uint64_t:
                return std::get<3>(_val) == std::get<3>(oth._val);
            case types::float32:
                return std::get<4>(_val) == std::get<4>(oth._val);
            case types::float64:
                return std::get<5>(_val) == std::get<5>(oth._val);
            default:
                throw std::invalid_argument("Unsupported type.");
        }
    }

    std::string to_string() {
        return std::visit([](auto&& arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::string>)
                return arg;
            else if constexpr (std::is_arithmetic_v<T>)
                return std::to_string(arg);
            else 
                throw std::exception("Can't convert given type to string.\n");
        }, _val);
    }

    // just for debuging
    friend std::ostream& operator<<(std::ostream& os, const db_val& db_val) {
        switch (static_cast<types>(db_val._val.index())) {
            case types::int32_t:
                os << std::get<0>(db_val._val);
                break;
            case types::uint32_t:
                os << std::get<1>(db_val._val);
                break;
            case types::int64_t:
                os << std::get<2>(db_val._val);
                break;
            case types::uint64_t:
                os << std::get<3>(db_val._val);
                break;
            case types::float32:
                os << std::get<4>(db_val._val);
                break;
            case types::float64:
                os << std::get<5>(db_val._val);
                break;
            default:
                throw std::invalid_argument("Unsupported type.");
        }

        return os;
    }
};

db_val get_value(int32_t* ptr, types type) {
    // db_value val;
    switch(type){
        case types::int32_t:
            return db_val(*reinterpret_cast<int32_t*>(ptr));
        case types::uint32_t:
            return db_val(*reinterpret_cast<uint32_t*>(ptr));
        case types::int64_t:
            return db_val(*reinterpret_cast<int64_t*>(ptr));
        case types::uint64_t:
            return db_val(*reinterpret_cast<uint64_t*>(ptr));
        case types::float32:
            return db_val(*reinterpret_cast<float*>(ptr));
        case types::float64:
            return db_val(*reinterpret_cast<double*>(ptr));
        default:
            throw std::invalid_argument("Unsupported type.");
    }
}

static db_val str_to_db_val(std::string& str_val, types type) {
     switch(type){
        case types::int32_t:
            return db_val(int32_t(std::stol(str_val)));
        case types::uint32_t:
            return db_val(uint32_t(std::stoul(str_val)));
        case types::int64_t:
            return db_val(int64_t(std::stoll(str_val)));
        case types::uint64_t:
            return db_val(uint64_t(std::stoull(str_val)));
        case types::float32:
            return db_val(std::stof(str_val));
        case types::float64:
            return db_val(std::stod(str_val));
        default:
            throw std::invalid_argument("Unsupported type.");
    }  
}

// mogu imati mapu na pointer na klasu koja ne radi nista
// iz nje mogu izvesti druge klase koje rade nesto zapravo