// log.h
#ifndef LOG_H
#define LOG_H

#include <string>
#include <deque>
#include <ctime>
#include <iostream>

// Уровни важности событий
enum LogLevel {
    LOG_NORMAL,
    LOG_WARNING,
    LOG_ERROR
};

// Класс-логгер (Singleton)
class Log {
public:
    // Запрещаем копирование и присваивание
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    // Единственная точка доступа
    static Log* Instance() {
        static Log instance;
        return &instance;
    }

    // Зафиксировать событие
    void message(LogLevel level, const std::string& message) {
        Entry entry;
        entry.timestamp = std::time(nullptr);
        entry.level = level;
        entry.message = message;
        events_.push_back(entry);
        // Оставляем только последние 10 записей
        if (events_.size() > 10)
            events_.pop_front();
    }

    // Вывести последние 10 событий
    void print() const {
        std::cout << "=== Last " << events_.size() << " events ===\n";
        for (const auto& e : events_) {
            std::cout << '[' << std::ctime(&e.timestamp) << "] ";

            switch (e.level) {
                case LOG_NORMAL:  std::cout << "[NORMAL] "; break;
                case LOG_WARNING: std::cout << "[WARNING] "; break;
                case LOG_ERROR:   std::cout << "[ERROR] "; break;
            }
            std::cout << e.message << std::endl;
        }
    }

private:
    // Приватный конструктор (Singleton)
    Log() = default;

    struct Entry {
        std::time_t timestamp;
        LogLevel level;
        std::string message;
    };

    std::deque<Entry> events_;   // хранит не более 10 записей
};

#endif // LOG_H
