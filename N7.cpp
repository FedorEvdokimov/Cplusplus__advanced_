#include <iostream>
#include <memory>
#include <unordered_set>
#include <vector>
#include <algorithm>

// ---------- Абстрактная реализация ----------
class SetImpl {
public:
    virtual ~SetImpl() = default;
    virtual void add(int elem) = 0;
    virtual void remove(int elem) = 0;
    virtual bool contains(int elem) const = 0;
    virtual size_t size() const = 0;
    virtual std::vector<int> getAllElements() const = 0; // для копирования при смене реализации
};

// ---------- Конкретная реализация 1: массив (для малых размеров) ----------
class ArraySetImpl : public SetImpl {
private:
    static constexpr size_t MAX_SIZE = 20;   // максимальный размер для этой реализации
    std::vector<int> data_;                  // линейное хранение
public:
    void add(int elem) override {
        if (contains(elem)) return;
        if (data_.size() < MAX_SIZE)
            data_.push_back(elem);
        else
            throw std::runtime_error("ArraySetImpl overflow: too many elements");
    }
    void remove(int elem) override {
        auto it = std::find(data_.begin(), data_.end(), elem);
        if (it != data_.end())
            data_.erase(it);
    }
    bool contains(int elem) const override {
        return std::find(data_.begin(), data_.end(), elem) != data_.end();
    }
    size_t size() const override { return data_.size(); }
    std::vector<int> getAllElements() const override { return data_; }
};

// ---------- Конкретная реализация 2: хэш-таблица (для больших размеров) ----------
class HashSetImpl : public SetImpl {
private:
    std::unordered_set<int> data_;
public:
    void add(int elem) override { data_.insert(elem); }
    void remove(int elem) override { data_.erase(elem); }
    bool contains(int elem) const override { return data_.find(elem) != data_.end(); }
    size_t size() const override { return data_.size(); }
    std::vector<int> getAllElements() const override {
        return std::vector<int>(data_.begin(), data_.end());
    }
};

// ---------- Абстракция: Множество (Bridge) ----------
class Set {
private:
    static constexpr size_t THRESHOLD = 20;   // порог переключения
    std::unique_ptr<SetImpl> impl_;

    // Вспомогательный метод для смены реализации с сохранением всех элементов
    void switchImplementation(std::unique_ptr<SetImpl> newImpl) {
        auto oldElements = impl_->getAllElements();
        for (int elem : oldElements)
            newImpl->add(elem);
        impl_ = std::move(newImpl);
    }

public:
    // Начальная реализация — массив
    Set() : impl_(std::make_unique<ArraySetImpl>()) {}

    void add(int elem) {
        impl_->add(elem);
        // Если превысили порог и текущая реализация — не хэш-таблица, переключаем
        if (impl_->size() > THRESHOLD && dynamic_cast<HashSetImpl*>(impl_.get()) == nullptr) {
            switchImplementation(std::make_unique<HashSetImpl>());
        }
    }

    void remove(int elem) {
        impl_->remove(elem);
        // Если размер стал не больше порога и текущая реализация — хэш-таблица, переключаем обратно
        if (impl_->size() <= THRESHOLD && dynamic_cast<ArraySetImpl*>(impl_.get()) == nullptr) {
            switchImplementation(std::make_unique<ArraySetImpl>());
        }
    }

    bool contains(int elem) const {
        return impl_->contains(elem);
    }

    size_t size() const {
        return impl_->size();
    }

    // Объединение множеств
    Set unionSet(const Set& other) const {
        Set result;
        for (int e : impl_->getAllElements()) result.add(e);
        for (int e : other.impl_->getAllElements()) result.add(e);
        return result;
    }

    // Пересечение множеств
    Set intersection(const Set& other) const {
        Set result;
        for (int e : impl_->getAllElements()) {
            if (other.contains(e))
                result.add(e);
        }
        return result;
    }

    // Для отладки: вывести все элементы
    void print() const {
        auto elems = impl_->getAllElements();
        std::cout << "{ ";
        for (int e : elems) std::cout << e << " ";
        std::cout << "}  (size=" << size() << ", impl="
                  << (dynamic_cast<ArraySetImpl*>(impl_.get()) ? "Array" : "HashSet") << ")\n";
    }
};

// ---------- Пример использования ----------
int main() {
    Set s;
    std::cout << "Initial: ";
    s.print();

    // Добавляем 25 элементов, должно переключиться на HashSetImpl
    for (int i = 1; i <= 25; ++i)
        s.add(i);
    std::cout << "After adding 25 elements: ";
    s.print();

    // Удаляем первые 10, размер 15 — должно вернуться на ArraySetImpl
    for (int i = 1; i <= 10; ++i)
        s.remove(i);
    std::cout << "After removing 10 elements: ";
    s.print();

    Set t;
    t.add(10);
    t.add(20);
    t.add(30);
    std::cout << "Set t: ";
    t.print();

    Set u = s.unionSet(t);
    std::cout << "Union s ∪ t: ";
    u.print();

    Set i = s.intersection(t);
    std::cout << "Intersection s ∩ t: ";
    i.print();

    return 0;
}
