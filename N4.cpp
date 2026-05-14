#include <iostream>
#include <cassert>

// Пространство имён для всех mixin-классов
namespace mixins {

    // 1) less_than_comparable: добавляет операторы сравнения, используя CRTP
    template<typename T>
    class less_than_comparable {
    public:
        // Все операторы выражаются через operator<, который должен быть определён в T
        friend bool operator>(const T& a, const T& b) { return b < a; }
        friend bool operator<=(const T& a, const T& b) { return !(b < a); }
        friend bool operator>=(const T& a, const T& b) { return !(a < b); }
        friend bool operator==(const T& a, const T& b) { return !(a < b) && !(b < a); }
        friend bool operator!=(const T& a, const T& b) { return !(a == b); }
    };

    // 2) counter: подсчёт количества живых объектов
    template<typename T>
    class counter {
    private:
        // static inline (C++17) – одна переменная на все экземпляры T
        static inline std::size_t count_ = 0;

    protected:
        // Конструкторы увеличивают счётчик (по умолчанию, копирования, перемещения)
        counter() { ++count_; }
        counter(const counter&) { ++count_; }
        counter(counter&&) noexcept { ++count_; }

        // Деструктор уменьшает счётчик
        ~counter() { --count_; }

    public:
        static std::size_t count() { return count_; }
    };

} // namespace mixins

// Пример использования: класс Number, наследующий оба mixin'а
class Number : public mixins::less_than_comparable<Number>,
               public mixins::counter<Number> {
public:
    explicit Number(int value) : m_value{value} {}

    int value() const { return m_value; }

    // Единственный оператор, который нужно реализовать вручную
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

    // Все операторы сравнения работают автоматически
    assert(one >= one);   // true
    assert(three <= four); // true
    assert(two == two);    // true
    assert(three > two);   // true
    assert(one < two);     // true

    // Подсчёт текущего количества объектов Number
    std::cout << "Count: " << mixins::counter<Number>::count() << std::endl;

    return 0;
}
