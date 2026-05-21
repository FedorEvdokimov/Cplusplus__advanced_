#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <memory>
#include <initializer_list>

// ==================================================================
// Паттерн Bridge: реализация множества с переключением структур
// ==================================================================

// ---------------------- Абстрактная реализация (Implementor) -----
class SetImpl {
public:
    virtual ~SetImpl() = default;
    virtual void add(int elem) = 0;
    virtual void remove(int elem) = 0;
    virtual bool contains(int elem) const = 0;
    virtual std::vector<int> getElements() const = 0;
    virtual size_t size() const = 0;
    virtual std::unique_ptr<SetImpl> clone() const = 0;
};

// ---------------------- Конкретная реализация: массив (для малых множеств) ---
class ArraySetImpl : public SetImpl {
    std::vector<int> data;
public:
    ArraySetImpl() = default;
    ArraySetImpl(const ArraySetImpl& other) : data(other.data) {}
    void add(int elem) override {
        if (!contains(elem)) {
            data.push_back(elem);
        }
    }
    void remove(int elem) override {
        auto it = std::find(data.begin(), data.end(), elem);
        if (it != data.end()) data.erase(it);
    }
    bool contains(int elem) const override {
        return std::find(data.begin(), data.end(), elem) != data.end();
    }
    std::vector<int> getElements() const override {
        return data;
    }
    size_t size() const override {
        return data.size();
    }
    std::unique_ptr<SetImpl> clone() const override {
        return std::make_unique<ArraySetImpl>(*this);
    }
};

// ---------------------- Конкретная реализация: хеш-таблица (для больших множеств) -
class HashSetImpl : public SetImpl {
    std::unordered_set<int> data;
public:
    HashSetImpl() = default;
    HashSetImpl(const HashSetImpl& other) : data(other.data) {}
    void add(int elem) override {
        data.insert(elem);
    }
    void remove(int elem) override {
        data.erase(elem);
    }
    bool contains(int elem) const override {
        return data.find(elem) != data.end();
    }
    std::vector<int> getElements() const override {
        return std::vector<int>(data.begin(), data.end());
    }
    size_t size() const override {
        return data.size();
    }
    std::unique_ptr<SetImpl> clone() const override {
        return std::make_unique<HashSetImpl>(*this);
    }
};

// ---------------------- Абстракция (Bridge) -------------------------
class Set {
    std::unique_ptr<SetImpl> impl;
    static constexpr size_t THRESHOLD = 10;   // переключаемся при 10 элементах

    void checkAndSwitch() {
        size_t sz = impl->size();
        if (sz < THRESHOLD) {
            // нужно переключиться на ArraySetImpl, если ещё не он
            if (dynamic_cast<HashSetImpl*>(impl.get()) != nullptr) {
                // был хеш, становися массивом
                auto elements = impl->getElements();
                auto newImpl = std::make_unique<ArraySetImpl>();
                for (int e : elements) newImpl->add(e);
                impl = std::move(newImpl);
            }
        } else {
            // нужно переключиться на HashSetImpl, если ещё не он
            if (dynamic_cast<ArraySetImpl*>(impl.get()) != nullptr) {
                auto elements = impl->getElements();
                auto newImpl = std::make_unique<HashSetImpl>();
                for (int e : elements) newImpl->add(e);
                impl = std::move(newImpl);
            }
        }
    }

public:
    Set() : impl(std::make_unique<ArraySetImpl>()) {}
    Set(const Set& other) : impl(other.impl->clone()) {}
    Set(Set&&) = default;
    Set& operator=(const Set& other) {
        if (this != &other) impl = other.impl->clone();
        return *this;
    }
    Set& operator=(Set&&) = default;

    void add(int elem) {
        impl->add(elem);
        checkAndSwitch();
    }
    void remove(int elem) {
        impl->remove(elem);
        checkAndSwitch();
    }
    bool contains(int elem) const {
        return impl->contains(elem);
    }
    size_t size() const {
        return impl->size();
    }
    std::vector<int> getElements() const {
        return impl->getElements();
    }

    // Объединение множеств
    Set unionWith(const Set& other) const {
        Set result;
        // добавляем элементы из текущего
        for (int e : this->getElements()) result.add(e);
        // добавляем элементы из другого
        for (int e : other.getElements()) result.add(e);
        return result;
    }

    // Пересечение множеств
    Set intersectWith(const Set& other) const {
        Set result;
        for (int e : this->getElements()) {
            if (other.contains(e)) result.add(e);
        }
        return result;
    }

    // Вывод на печать (для демонстрации)
    void print() const {
        std::cout << "{ ";
        for (int e : getElements()) std::cout << e << " ";
        std::cout << "}";
    }
};

// ==================================================================
// Тестирование
// ==================================================================
int main() {
    Set a;
    for (int i = 1; i <= 15; ++i) {
        a.add(i);
    }
    std::cout << "Set a (15 elements): ";
    a.print();
    std::cout << ", size = " << a.size() << "\n";

    Set b;
    for (int i = 10; i <= 20; ++i) {
        b.add(i);
    }
    std::cout << "Set b (11 elements): ";
    b.print();
    std::cout << ", size = " << b.size() << "\n";

    Set c = a.unionWith(b);
    std::cout << "Union a ∪ b: ";
    c.print();
    std::cout << ", size = " << c.size() << "\n";

    Set d = a.intersectWith(b);
    std::cout << "Intersection a ∩ b: ";
    d.print();
    std::cout << ", size = " << d.size() << "\n";

    // Проверка переключения: добавим много элементов в маленькое множество
    Set small;
    for (int i = 1; i <= 5; ++i) small.add(i);
    std::cout << "\nSmall set (5 elements): ";
    small.print();
    std::cout << " (should be ArraySetImpl)\n";
    for (int i = 6; i <= 12; ++i) small.add(i);
    std::cout << "After adding up to 12 elements: ";
    small.print();
    std::cout << " (now should be HashSetImpl)\n";

    // Удаляем, чтобы снова стал маленьким
    for (int i = 10; i <= 12; ++i) small.remove(i);
    std::cout << "After removing 10-12: ";
    small.print();
    std::cout << " (should switch back to ArraySetImpl)\n";

    return 0;
}
