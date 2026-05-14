#include <iostream>
#include <map>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>

// Forward declaration
class ExpressionFactory;

// ---------- Базовый класс узла выражения ----------
class Expression {
public:
    virtual ~Expression() = default;
    virtual void print(std::ostream& os) const = 0;
    virtual int calculate(const std::map<std::string, int>& context) const = 0;
    // Для определения типа (нужно фабрике при удалении)
    enum class Type { Constant, Variable, Add, Sub, Mul, Div };
    virtual Type getType() const = 0;
};

// ---------- Приспособленец: Константа ----------
class Constant : public Expression {
    int value_;
    // Конструктор и деструктор приватны – создаются и удаляются только фабрикой
    Constant(int val) : value_(val) {}
    ~Constant() = default;
public:
    Type getType() const override { return Type::Constant; }
    void print(std::ostream& os) const override { os << value_; }
    int calculate(const std::map<std::string, int>&) const override { return value_; }
    int getValue() const { return value_; }
    friend class ExpressionFactory;
};

// ---------- Приспособленец: Переменная ----------
class Variable : public Expression {
    std::string name_;
    Variable(const std::string& name) : name_(name) {}
    ~Variable() = default;
public:
    Type getType() const override { return Type::Variable; }
    void print(std::ostream& os) const override { os << name_; }
    int calculate(const std::map<std::string, int>& context) const override {
        auto it = context.find(name_);
        if (it == context.end())
            throw std::runtime_error("Variable " + name_ + " not found in context");
        return it->second;
    }
    const std::string& getName() const { return name_; }
    friend class ExpressionFactory;
};

// ---------- Композитные узлы: операторы ----------
class BinaryOperator : public Expression {
protected:
    Expression* left_;
    Expression* right_;
public:
    BinaryOperator(Expression* left, Expression* right) : left_(left), right_(right) {}
    ~BinaryOperator() {
        // Освобождаем операнды через фабрику (см. ниже)
        // Фабрика будет вызвана из деструкторов конкретных операторов,
        // но т.к. фабрика – глобальный объект, мы можем её получить.
        // Для простоты вынесем вызов в конкретные классы.
    }
};

class Addition : public BinaryOperator {
public:
    Addition(Expression* left, Expression* right) : BinaryOperator(left, right) {}
    Type getType() const override { return Type::Add; }
    void print(std::ostream& os) const override {
        os << "("; left_->print(os); os << " + "; right_->print(os); os << ")";
    }
    int calculate(const std::map<std::string, int>& ctx) const override {
        return left_->calculate(ctx) + right_->calculate(ctx);
    }
    ~Addition() {
        // Удаляем операнды через фабрику (нужен доступ к фабрике)
        extern ExpressionFactory& getExpressionFactory(); // объявление
        getExpressionFactory().destroy(left_);
        getExpressionFactory().destroy(right_);
    }
};

class Subtraction : public BinaryOperator {
public:
    Subtraction(Expression* left, Expression* right) : BinaryOperator(left, right) {}
    Type getType() const override { return Type::Sub; }
    void print(std::ostream& os) const override {
        os << "("; left_->print(os); os << " - "; right_->print(os); os << ")";
    }
    int calculate(const std::map<std::string, int>& ctx) const override {
        return left_->calculate(ctx) - right_->calculate(ctx);
    }
    ~Subtraction() {
        extern ExpressionFactory& getExpressionFactory();
        getExpressionFactory().destroy(left_);
        getExpressionFactory().destroy(right_);
    }
};

class Multiplication : public BinaryOperator {
public:
    Multiplication(Expression* left, Expression* right) : BinaryOperator(left, right) {}
    Type getType() const override { return Type::Mul; }
    void print(std::ostream& os) const override {
        os << "("; left_->print(os); os << " * "; right_->print(os); os << ")";
    }
    int calculate(const std::map<std::string, int>& ctx) const override {
        return left_->calculate(ctx) * right_->calculate(ctx);
    }
    ~Multiplication() {
        extern ExpressionFactory& getExpressionFactory();
        getExpressionFactory().destroy(left_);
        getExpressionFactory().destroy(right_);
    }
};

class Division : public BinaryOperator {
public:
    Division(Expression* left, Expression* right) : BinaryOperator(left, right) {}
    Type getType() const override { return Type::Div; }
    void print(std::ostream& os) const override {
        os << "("; left_->print(os); os << " / "; right_->print(os); os << ")";
    }
    int calculate(const std::map<std::string, int>& ctx) const override {
        int denom = right_->calculate(ctx);
        if (denom == 0) throw std::runtime_error("Division by zero");
        return left_->calculate(ctx) / denom;
    }
    ~Division() {
        extern ExpressionFactory& getExpressionFactory();
        getExpressionFactory().destroy(left_);
        getExpressionFactory().destroy(right_);
    }
};

// ---------- Фабрика для создания/удаления приспособленцев ----------
class ExpressionFactory {
private:
    // Предсозданные константы от -5 до 256 (включительно)
    static constexpr int MIN_PRECREATED = -5;
    static constexpr int MAX_PRECREATED = 256;
    Constant* precreatedConstants_[MAX_PRECREATED - MIN_PRECREATED + 1]{};

    // Динамические константы (вне диапазона)
    std::map<int, std::pair<Constant*, int>> dynamicConstants_; // value -> (ptr, refcount)

    // Переменные
    std::map<std::string, std::pair<Variable*, int>> variables_; // name -> (ptr, refcount)

    void initPrecreated() {
        for (int i = MIN_PRECREATED; i <= MAX_PRECREATED; ++i) {
            precreatedConstants_[i - MIN_PRECREATED] = new Constant(i);
        }
    }

public:
    ExpressionFactory() {
        initPrecreated();
    }

    ~ExpressionFactory() {
        // Удаляем предсозданные
        for (auto* c : precreatedConstants_) delete c;
        // Удаляем динамические константы (должны иметь нулевой счётчик)
        for (auto& p : dynamicConstants_) delete p.second.first;
        // Удаляем переменные (счётчик должен быть 0)
        for (auto& p : variables_) delete p.second.first;
    }

    Constant* createConstant(int value) {
        // Предсозданный диапазон?
        if (value >= MIN_PRECREATED && value <= MAX_PRECREATED) {
            return precreatedConstants_[value - MIN_PRECREATED];
        }
        // Динамическая константа
        auto it = dynamicConstants_.find(value);
        if (it != dynamicConstants_.end()) {
            it->second.second++; // увеличиваем счётчик
            return it->second.first;
        } else {
            Constant* c = new Constant(value);
            dynamicConstants_[value] = {c, 1};
            return c;
        }
    }

    void destroyConstant(Constant* c) {
        int val = c->getValue();
        if (val >= MIN_PRECREATED && val <= MAX_PRECREATED) {
            // Предсозданные никогда не удаляются – ничего не делаем
            return;
        }
        auto it = dynamicConstants_.find(val);
        if (it == dynamicConstants_.end())
            throw std::runtime_error("Destroying unknown constant");
        it->second.second--;
        if (it->second.second == 0) {
            delete it->second.first;
            dynamicConstants_.erase(it);
        }
    }

    Variable* createVariable(const std::string& name) {
        auto it = variables_.find(name);
        if (it != variables_.end()) {
            it->second.second++;
            return it->second.first;
        } else {
            Variable* v = new Variable(name);
            variables_[name] = {v, 1};
            return v;
        }
    }

    void destroyVariable(Variable* v) {
        const std::string& name = v->getName();
        auto it = variables_.find(name);
        if (it == variables_.end())
            throw std::runtime_error("Destroying unknown variable");
        it->second.second--;
        if (it->second.second == 0) {
            delete it->second.first;
            variables_.erase(it);
        }
    }

    // Универсальный метод удаления узла выражения
    void destroy(Expression* e) {
        if (!e) return;
        switch (e->getType()) {
            case Expression::Type::Constant:
                destroyConstant(static_cast<Constant*>(e));
                break;
            case Expression::Type::Variable:
                destroyVariable(static_cast<Variable*>(e));
                break;
            default:
                // Операторы не управляются фабрикой, но они владеют операндами,
                // которые уже удалили в своих деструкторах. Однако сам оператор
                // должен быть удален через delete пользователем.
                // Ничего не делаем.
                break;
        }
    }
};

// Глобальная фабрика (синглтон) – для доступа из деструкторов операторов
static ExpressionFactory& getExpressionFactory() {
    static ExpressionFactory factory;
    return factory;
}

// ---------- Пример использования ----------
int main() {
    ExpressionFactory& factory = getExpressionFactory();

    // Создаём выражение: 2 + x
    Constant* c = factory.createConstant(2);
    Variable* v = factory.createVariable("x");
    Addition* expr = new Addition(c, v);

    std::map<std::string, int> context;
    context["x"] = 3;

    std::cout << "Expression: ";
    expr->print(std::cout);
    std::cout << std::endl;
    std::cout << "Result: " << expr->calculate(context) << std::endl;

    delete expr;  // деструктор Addition вызовет factory.destroy для операндов
    // c и v больше не используются, их счётчики станут 0 и они удалятся

    return 0;
}
