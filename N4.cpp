#include <iostream>
#include <cassert>

// ==============================
// MixIn: less_than_comparable
// ==============================
template<typename Derived>
class less_than_comparable {
public:
    friend bool operator>(const Derived& lhs, const Derived& rhs) {
        return rhs < lhs;
    }
    friend bool operator<=(const Derived& lhs, const Derived& rhs) {
        return !(rhs < lhs);
    }
    friend bool operator>=(const Derived& lhs, const Derived& rhs) {
        return !(lhs < rhs);
    }
    friend bool operator==(const Derived& lhs, const Derived& rhs) {
        return !(lhs < rhs) && !(rhs < lhs);
    }
    friend bool operator!=(const Derived& lhs, const Derived& rhs) {
        return !(lhs == rhs);
    }
};

// ==============================
// MixIn: counter (counts live instances)
// ==============================
template<typename Derived>
class counter {
private:
    static inline int live_count = 0;

protected:
    counter() {
        ++live_count;
    }
    counter(const counter&) {
        ++live_count;
    }
    counter(counter&&) noexcept {
        ++live_count;
    }
    ~counter() {
        --live_count;
    }

public:
    static int count() {
        return live_count;
    }
};

// ==============================
// Example usage
// ==============================
class Number : public less_than_comparable<Number>, public counter<Number> {
public:
    Number(int value) : m_value(value) {}

    int value() const { return m_value; }

    bool operator<(const Number& other) const {
        return m_value < other.m_value;
    }

private:
    int m_value;
};

int main() {
    Number one{1};
    Number two{2};
    Number three{3};
    Number four{4};

    assert(one >= one);
    assert(three <= four);
    assert(two == two);
    assert(three > two);
    assert(one < two);

    std::cout << "Count: " << counter<Number>::count() << std::endl; // Output: 4

    return 0;
}
