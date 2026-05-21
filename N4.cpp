#include <iostream>
#include <cassert>

// ==================================================================
// MixIn: less_than_comparable
// ==================================================================
// Принцип: использует CRTP (Curiously Recurring Template Pattern),
// чтобы "подмешать" в целевой класс операторы сравнения, имея только operator<.
// Все дружественные функции реализованы через operator<.
// ==================================================================
template<typename Derived>
class less_than_comparable {
public:
    // a > b   <=>   b < a
    friend bool operator>(const Derived& lhs, const Derived& rhs) {
        return rhs < lhs;
    }
    // a <= b   <=>   не (b < a)
    friend bool operator<=(const Derived& lhs, const Derived& rhs) {
        return !(rhs < lhs);
    }
    // a >= b   <=>   не (a < b)
    friend bool operator>=(const Derived& lhs, const Derived& rhs) {
        return !(lhs < rhs);
    }
    // a == b   <=>   не (a < b) и не (b < a)
    friend bool operator==(const Derived& lhs, const Derived& rhs) {
        return !(lhs < rhs) && !(rhs < lhs);
    }
    // a != b   <=>   не (a == b)
    friend bool operator!=(const Derived& lhs, const Derived& rhs) {
        return !(lhs == rhs);
    }
};

// ==================================================================
// MixIn: counter (подсчёт живых экземпляров)
// ==================================================================
// Принцип: использует CRTP и статическую переменную, уникальную для каждого
// типа Derived благодаря шаблону. Счётчик увеличивается в конструкторах и
// уменьшается в деструкторе. static inline (C++17) гарантирует
// единственный экземпляр переменной на всю программу.
// ==================================================================
template<typename Derived>
class counter {
private:
    // static inline переменная – инициализируется один раз для каждого Derived
    static inline int live_count = 0;

protected:
    // Конструктор по умолчанию
    counter() {
        ++live_count;
    }
    // Конструктор копирования
    counter(const counter&) {
        ++live_count;
    }
    // Конструктор перемещения
    counter(counter&&) noexcept {
        ++live_count;
    }
    // Деструктор
    ~counter() {
        --live_count;
    }

public:
    // Статический метод для получения текущего количества живых объектов
    static int count() {
        return live_count;
    }
};

// ==================================================================
// Пример класса, использующего оба MixIn
// ==================================================================
class Number : public less_than_comparable<Number>, public counter<Number> {
public:
    Number(int value) : m_value(value) {}

    int value() const { return m_value; }

    // Единственный оператор, который нужно определить – operator<
    bool operator<(const Number& other) const {
        return m_value < other.m_value;
    }

private:
    int m_value;
};

// ==================================================================
// Тестирование
// ==================================================================
int main() {
    Number one{1};
    Number two{2};
    Number three{3};
    Number four{4};

    // Все операторы сравнения работают, хотя определён только operator<
    assert(one >= one);   // >=
    assert(three <= four);// <=
    assert(two == two);   // ==
    assert(three > two);  // >
    assert(one < two);    // <

    // Подсчёт живых объектов
    std::cout << "Count: " << counter<Number>::count() << std::endl; // Вывод: 4

    return 0;
}
