// main.cpp – пример использования
#include "log.h"

int main() {
    Log* log = Log::Instance();
    log->message(LOG_NORMAL, "program loaded");
    log->message(LOG_WARNING, "low memory warning");
    log->message(LOG_ERROR,   "error happens! help me!");
    log->message(LOG_NORMAL,  "step 1 completed");
    log->message(LOG_NORMAL,  "step 2 completed");
    // ... можно добавить ещё сообщений, но выведутся только последние 10
    log->print();
    return 0;
}
