#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <limits>

// ==================================================================
// Классы контрольных пунктов
// ==================================================================

// Базовый класс для всех КП
class ControlPoint {
protected:
    std::string name;
    double latitude;   // широта: -90..90
    double longitude;  // долгота: -180..180
public:
    ControlPoint(const std::string& n, double lat, double lon)
        : name(n), latitude(lat), longitude(lon) {
        // Валидация координат
        if (lat < -90.0 || lat > 90.0)
            throw std::out_of_range("Latitude must be in [-90, 90]");
        if (lon < -180.0 || lon > 180.0)
            throw std::out_of_range("Longitude must be in [-180, 180]");
    }
    virtual ~ControlPoint() = default;

    const std::string& getName() const { return name; }
    double getLatitude() const { return latitude; }
    double getLongitude() const { return longitude; }

    // Чисто виртуальные методы для посещения (Visitor-like для Builder)
    virtual void accept(class Builder& builder) const = 0;
};

// Обязательный КП
class MandatoryCP : public ControlPoint {
public:
    MandatoryCP(const std::string& name, double lat, double lon)
        : ControlPoint(name, lat, lon) {}
    void accept(Builder& builder) const override;
};

// Необязательный КП (со штрафом)
class OptionalCP : public ControlPoint {
    double penalty; // штраф в часах
public:
    OptionalCP(const std::string& name, double lat, double lon, double p)
        : ControlPoint(name, lat, lon), penalty(p) {
        if (p < 0) throw std::out_of_range("Penalty cannot be negative");
    }
    double getPenalty() const { return penalty; }
    void accept(Builder& builder) const override;
};

// ==================================================================
// Абстрактный Builder
// ==================================================================
class Builder {
public:
    virtual ~Builder() = default;
    virtual void buildHeader() = 0;
    virtual void buildFooter() = 0;
    virtual void buildMandatoryCP(int index, const MandatoryCP& cp) = 0;
    virtual void buildOptionalCP(int index, const OptionalCP& cp) = 0;
    virtual void getResult() = 0;
};

// ==================================================================
// ConcreteBuilder для текстового вывода (на английском)
// ==================================================================
class TextBuilder : public Builder {
    std::ostringstream oss;
public:
    void buildHeader() override {
        oss << "List of Checkpoints:\n";
        oss << "------------------------------------------------------------\n";
        oss << std::left << std::setw(5) << "No"
            << std::setw(15) << "Name"
            << std::setw(15) << "Latitude"
            << std::setw(15) << "Longitude"
            << std::setw(15) << "Penalty/Fail\n";
        oss << "------------------------------------------------------------\n";
    }
    void buildFooter() override {
        oss << "------------------------------------------------------------\n";
    }
    void buildMandatoryCP(int index, const MandatoryCP& cp) override {
        oss << std::left << std::setw(5) << index
            << std::setw(15) << cp.getName()
            << std::setw(15) << cp.getLatitude()
            << std::setw(15) << cp.getLongitude()
            << std::setw(15) << "FAIL\n";
    }
    void buildOptionalCP(int index, const OptionalCP& cp) override {
        oss << std::left << std::setw(5) << index
            << std::setw(15) << cp.getName()
            << std::setw(15) << cp.getLatitude()
            << std::setw(15) << cp.getLongitude()
            << std::setw(15) << (std::to_string(cp.getPenalty()) + " h")
            << "\n";
    }
    void getResult() override {
        std::cout << oss.str();
    }
};

// ==================================================================
// ConcreteBuilder для подсчёта суммарного штрафа (вывод на английском)
// ==================================================================
class PenaltyBuilder : public Builder {
    double totalPenalty = 0.0;
public:
    void buildHeader() override {}
    void buildFooter() override {}
    void buildMandatoryCP(int, const MandatoryCP&) override {}
    void buildOptionalCP(int, const OptionalCP& cp) override {
        totalPenalty += cp.getPenalty();
    }
    void getResult() override {
        std::cout << "Total penalty for optional CPs: "
                  << totalPenalty << " hours\n";
    }
    double getTotal() const { return totalPenalty; }
};

// ==================================================================
// ConcreteBuilder для вывода в таблицу (имитация QTableView, на английском)
// ==================================================================
class TableBuilder : public Builder {
    struct Row {
        int index;
        std::string name;
        double lat, lon;
        std::string penaltyStr;
    };
    std::vector<Row> rows;
public:
    void buildHeader() override { rows.clear(); }
    void buildFooter() override {
        std::cout << "\n=== Checkpoint Table (similar to QTableView) ===\n";
        std::cout << std::left << std::setw(5) << "No"
                  << std::setw(15) << "Name"
                  << std::setw(15) << "Latitude"
                  << std::setw(15) << "Longitude"
                  << std::setw(15) << "Penalty/Fail\n";
        std::cout << "------------------------------------------------------------\n";
        for (const auto& row : rows) {
            std::cout << std::left << std::setw(5) << row.index
                      << std::setw(15) << row.name
                      << std::setw(15) << row.lat
                      << std::setw(15) << row.lon
                      << std::setw(15) << row.penaltyStr << "\n";
        }
        std::cout << "------------------------------------------------------------\n";
    }
    void buildMandatoryCP(int index, const MandatoryCP& cp) override {
        rows.push_back({index, cp.getName(), cp.getLatitude(), cp.getLongitude(), "FAIL"});
    }
    void buildOptionalCP(int index, const OptionalCP& cp) override {
        rows.push_back({index, cp.getName(), cp.getLatitude(), cp.getLongitude(),
                        std::to_string(cp.getPenalty()) + " h"});
    }
    void getResult() override {}
};

// ==================================================================
// Реализация метода accept (после определения Builder)
// ==================================================================
void MandatoryCP::accept(Builder& builder) const {
    // Для простоты индекс будет передан Director'ом
}
void OptionalCP::accept(Builder& builder) const {
    // аналогично
}

// ==================================================================
// Director (Управляет построением)
// ==================================================================
class RaceDirector {
    std::vector<std::unique_ptr<ControlPoint>> checkpoints;
public:
    void addCP(std::unique_ptr<ControlPoint> cp) {
        checkpoints.push_back(std::move(cp));
    }

    void construct(Builder& builder) {
        builder.buildHeader();
        int index = 1;
        for (const auto& cp : checkpoints) {
            if (auto* m = dynamic_cast<MandatoryCP*>(cp.get())) {
                builder.buildMandatoryCP(index, *m);
            } else if (auto* o = dynamic_cast<OptionalCP*>(cp.get())) {
                builder.buildOptionalCP(index, *o);
            }
            ++index;
        }
        builder.buildFooter();
        builder.getResult();
    }
};

// ==================================================================
// Пример использования
// ==================================================================
int main() {
    try {
        RaceDirector director;

        // Добавляем КП
        director.addCP(std::make_unique<MandatoryCP>("Start", 55.0, 37.0));
        director.addCP(std::make_unique<OptionalCP>("Forest", 55.2, 37.5, 2.5));
        director.addCP(std::make_unique<MandatoryCP>("River", 55.4, 37.8));
        director.addCP(std::make_unique<OptionalCP>("Mountain", 55.6, 38.0, 5.0));
        director.addCP(std::make_unique<OptionalCP>("Finish", 55.8, 38.3, 0.0));

        // 1. Текстовый вывод
        TextBuilder textBuilder;
        director.construct(textBuilder);

        // 2. Подсчёт суммарного штрафа
        PenaltyBuilder penaltyBuilder;
        director.construct(penaltyBuilder);

        // 3. Вывод в виде таблицы
        TableBuilder tableBuilder;
        director.construct(tableBuilder);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
