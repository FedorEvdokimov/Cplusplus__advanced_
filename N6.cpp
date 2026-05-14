#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <memory>

// ---------- Модель данных ----------
enum class CPType { Mandatory, Optional };

class ControlPoint {
public:
    ControlPoint(std::string name, double lat, double lon)
        : name_(std::move(name)), latitude_(lat), longitude_(lon) {}
    virtual ~ControlPoint() = default;

    const std::string& getName() const { return name_; }
    double getLatitude() const { return latitude_; }
    double getLongitude() const { return longitude_; }
    virtual CPType getType() const = 0;
    virtual double getPenalty() const { return 0.0; } // для обязательных - 0

private:
    std::string name_;
    double latitude_;
    double longitude_;
};

class MandatoryCP : public ControlPoint {
public:
    MandatoryCP(std::string name, double lat, double lon)
        : ControlPoint(std::move(name), lat, lon) {}
    CPType getType() const override { return CPType::Mandatory; }
};

class OptionalCP : public ControlPoint {
public:
    OptionalCP(std::string name, double lat, double lon, double penalty)
        : ControlPoint(std::move(name), lat, lon), penalty_(penalty) {}
    CPType getType() const override { return CPType::Optional; }
    double getPenalty() const override { return penalty_; }

private:
    double penalty_;
};

// ---------- Builder (абстрактный) ----------
class CPPointBuilder {
public:
    virtual ~CPPointBuilder() = default;
    virtual void addMandatory(int index, const MandatoryCP& cp) = 0;
    virtual void addOptional(int index, const OptionalCP& cp) = 0;
    virtual std::string getResult() = 0; // результат в виде строки
};

// ---------- ConcreteBuilder 1: текстовый вывод ----------
class TextOutputBuilder : public CPPointBuilder {
    std::ostringstream output;
public:
    void addMandatory(int index, const MandatoryCP& cp) override {
        output << index << ". " << cp.getName() << "; "
               << std::fixed << std::setprecision(6)
               << cp.getLatitude() << ", " << cp.getLongitude()
               << "; незачёт СУ\n";
    }

    void addOptional(int index, const OptionalCP& cp) override {
        output << index << ". " << cp.getName() << "; "
               << std::fixed << std::setprecision(6)
               << cp.getLatitude() << ", " << cp.getLongitude()
               << "; штраф " << cp.getPenalty() << " ч\n";
    }

    std::string getResult() override {
        return output.str();
    }
};

// ---------- ConcreteBuilder 2: подсчёт суммарного штрафа ----------
class PenaltySumBuilder : public CPPointBuilder {
    double totalPenalty = 0.0;
public:
    void addMandatory(int, const MandatoryCP&) override {
        // ничего не делаем
    }
    void addOptional(int, const OptionalCP& cp) override {
        totalPenalty += cp.getPenalty();
    }
    std::string getResult() override {
        std::ostringstream oss;
        oss << "Total penalty: " << totalPenalty << " hours";
        return oss.str();
    }
};

// ---------- Функция обработки (Director) ----------
std::string processControlPoints(const std::vector<std::unique_ptr<ControlPoint>>& points,
                                 CPPointBuilder& builder) {
    int idx = 1;
    for (const auto& cp : points) {
        if (cp->getType() == CPType::Mandatory) {
            builder.addMandatory(idx, static_cast<const MandatoryCP&>(*cp));
        } else {
            builder.addOptional(idx, static_cast<const OptionalCP&>(*cp));
        }
        ++idx;
    }
    return builder.getResult();
}

// ---------- Пример использования ----------
int main() {
    std::vector<std::unique_ptr<ControlPoint>> route;
    route.push_back(std::make_unique<MandatoryCP>("Start", 55.751244, 37.618423));
    route.push_back(std::make_unique<OptionalCP>("Forest crossing", 55.812345, 37.700000, 1.5));
    route.push_back(std::make_unique<MandatoryCP>("Lake", 55.870000, 37.650000));
    route.push_back(std::make_unique<OptionalCP>("Mountain pass", 55.900000, 37.720000, 2.0));

    TextOutputBuilder textBuilder;
    std::string textReport = processControlPoints(route, textBuilder);
    std::cout << "=== Text report ===\n" << textReport;

    PenaltySumBuilder penaltyBuilder;
    std::string penaltyReport = processControlPoints(route, penaltyBuilder);
    std::cout << "=== Penalty report ===\n" << penaltyReport << std::endl;

    return 0;
}
