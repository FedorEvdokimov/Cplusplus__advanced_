#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <ctime>

// ==================================================================
// Уровни важности событий
// ==================================================================
enum LogLevel {
    LOG_NORMAL,   // нормальное сообщение
    LOG_WARNING,  // предупреждение
    LOG_ERROR     // ошибка
};

// ==================================================================
// Структура для хранения одного события
// ==================================================================
struct LogEntry {
    std::chrono::system_clock::time_point timestamp;   // время события
    LogLevel level;                                     // важность
    std::string message;                                // текст

    // Получить строковое представление уровня важности
    std::string levelToString() const {
        switch (level) {
            case LOG_NORMAL: return "NORMAL";
            case LOG_WARNING: return "WARNING";
            case LOG_ERROR:   return "ERROR";
            default: return "UNKNOWN";
        }
    }

    // Получить строковое представление времени (локальное)
    std::string timeToString() const {
        std::time_t tt = std::chrono::system_clock::to_time_t(timestamp);
        std::tm* tm = std::localtime(&tt);
        std::ostringstream oss;
        oss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
};

// ==================================================================
// Класс логгера (Singleton)
// ==================================================================
// Принцип Singleton:
//   - Приватный конструктор – запрет создания объектов вне класса.
//   - Статический метод Instance() возвращает ссылку на единственный
//     экземпляр, создавая его при первом вызове (ленивая инициализация).
//   - Удалены конструктор копирования и оператор присваивания.
// ==================================================================
class Log {
private:
    std::vector<LogEntry> entries;   // хранилище всех событий (можно ограничить размер, но оставим все)
    static const size_t MAX_PRINT = 10;  // количество последних событий для вывода

    // Приватный конструктор
    Log() {}

    // Запрет копирования и присваивания
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

public:
    // Статический метод доступа к единственному экземпляру
    static Log* Instance() {
        static Log instance;   // гарантированно создаётся один раз (thread-safe в C++11+)
        return &instance;
    }

    // Добавление события
    void message(LogLevel level, const std::string& msg) {
        LogEntry entry;
        entry.timestamp = std::chrono::system_clock::now();
        entry.level = level;
        entry.message = msg;
        entries.push_back(entry);
    }

    // Печать 10 последних событий (от старого к новому среди последних 10)
    void print() const {
        size_t total = entries.size();
        size_t start = (total > MAX_PRINT) ? total - MAX_PRINT : 0;
        size_t count = total - start;   // сколько напечатаем (≤ MAX_PRINT)

        std::cout << "\n===== Last " << count << " log entries =====\n";
        for (size_t i = start; i < total; ++i) {
            const auto& e = entries[i];
            std::cout << "[" << e.timeToString() << "] "
                      << "[" << e.levelToString() << "] "
                      << e.message << std::endl;
        }
        std::cout << "==================================\n";
    }

    // (Опционально) получить все события – для тестирования
    size_t size() const { return entries.size(); }
};

// ==================================================================
// Пример использования
// ==================================================================
int main() {
    Log* log = Log::Instance();

    log->message(LOG_NORMAL, "program loaded");
    log->message(LOG_NORMAL, "initialization complete");
    log->message(LOG_WARNING, "low disk space");
    log->message(LOG_ERROR, "network connection lost");
    log->message(LOG_NORMAL, "reconnecting...");
    log->message(LOG_ERROR, "error happens! help me!");

    // Добавим ещё несколько, чтобы проверить вывод 10 последних
    for (int i = 1; i <= 5; ++i) {
        log->message(LOG_NORMAL, "dummy message " + std::to_string(i));
    }

    log->print();

    return 0;
}
