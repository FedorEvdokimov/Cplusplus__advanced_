#include <iostream>
#include <cassert>

namespace mixins {

template<typename T>
class less_than_comparable {
public:
    friend bool operator>(const T& a, const T& b) { return b < a; }
    friend bool operator<=(const T& a, const T& b) { return !(b < a); }
    friend bool operator>=(const T& a, const T& b) { return !(a < b); }
    friend bool operator==(const T& a, const T& b) { return !(a < b) && !(b < a); }
    friend bool operator!=(const T& a, const T& b) { return !(a == b); }
};

template<typename T>
class counter {
private:
    static inline std::size_t count_ = 0;
protected:
    counter() { ++count_; }
    counter(const counter&) { ++count_; }
    counter(counter&&) noexcept { ++count_; }
    ~counter() { --count_; }
public:
    static std::size_t count() { return count_; }
};

} // namespace mixins

class Number : public mixins::less_than_comparable<Number>,
               public mixins::counter<Number> {
public:
    explicit Number(int value) : m_value{value} {}
    int value() const { return m_value; }
    bool operator<(const Number& other) const {
        return m_value < other.m_value;
    }
private:
    int m_value;
};

int main() {
    Number one{1}, two{2}, three{3}, four{4};
    assert(one >= one);
    assert(three <= four);
    assert(two == two);
    assert(three > two);
    assert(one < two);
    std::cout << "Count: " << mixins::counter<Number>::count() << std::endl;
    return 0;
}
