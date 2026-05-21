// ============================================================================
// ЗАДАЧА: Система управления пользователями и группами
// ТРЕБУЕМЫЕ ПРИНЦИПЫ:
// 1. Отсутствие циклических зависимостей (forward declaration + weak_ptr + отложенное определение)
// 2. RAII (умные указатели)
// 3. Инкапсуляция
// 4. Порядок компиляции: методы, использующие Group, определены ПОСЛЕ определения Group
// ============================================================================

#include <iostream>
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>

// Forward declaration (нужен только для объявления указателя/ссылки)
class Group;

// ============================================================================
// ОБЪЯВЛЕНИЕ класса User (только заголовки методов)
// ============================================================================
class User {
private:
    int id;
    std::string name;
    std::string email;
    std::weak_ptr<Group> group_weak;  // weak_ptr разрывает цикл

public:
    User(int uid, const std::string& uname, const std::string& uemail = "")
        : id(uid), name(uname), email(uemail) {}

    int getId() const { return id; }
    std::string getName() const { return name; }
    std::string getEmail() const { return email; }

    void setGroup(std::shared_ptr<Group> grp);
    std::shared_ptr<Group> getGroup() const;

    // Только ОБЪЯВЛЕНИЕ. Определение будет ПОСЛЕ класса Group.
    void printInfo() const;
};

// ============================================================================
// ОПРЕДЕЛЕНИЕ класса Group
// ============================================================================
class Group {
private:
    int id;
    std::vector<int> userIds;

public:
    Group(int gid) : id(gid) {}

    int getId() const { return id; }

    void addUser(int userId) {
        if (std::find(userIds.begin(), userIds.end(), userId) == userIds.end()) {
            userIds.push_back(userId);
        }
    }

    void removeUser(int userId) {
        auto it = std::find(userIds.begin(), userIds.end(), userId);
        if (it != userIds.end()) userIds.erase(it);
    }

    const std::vector<int>& getUserIds() const { return userIds; }

    // Вывод информации о группе (требует доступа к хранилищу пользователей)
    void printInfo(const std::unordered_map<int, std::shared_ptr<User>>& userStore) const;

    void printInfoShort() const {
        std::cout << "Group{id=" << id << ", user_count=" << userIds.size() << "}" << std::endl;
    }
};

// ============================================================================
// ОПРЕДЕЛЕНИЯ методов User (теперь класс Group полностью известен)
// ============================================================================

void User::setGroup(std::shared_ptr<Group> grp) {
    group_weak = grp;
}

std::shared_ptr<Group> User::getGroup() const {
    return group_weak.lock();  // lock() возвращает shared_ptr, если объект ещё жив
}

void User::printInfo() const {
    std::cout << "User{id=" << id << ", name='" << name
              << "', email='" << email << "'";
    auto grp = getGroup();
    if (grp) {
        // Теперь компилятор знает, что такое Group и его метод getId()
        std::cout << ", group_id=" << grp->getId();
    } else {
        std::cout << ", group=null";
    }
    std::cout << "}" << std::endl;
}

// ============================================================================
// ОПРЕДЕЛЕНИЯ методов Group (требуют доступа к User)
// ============================================================================

void Group::printInfo(const std::unordered_map<int, std::shared_ptr<User>>& userStore) const {
    std::cout << "Group{id=" << id << ", users=[" << std::endl;
    for (int uid : userIds) {
        auto it = userStore.find(uid);
        if (it != userStore.end() && it->second) {
            std::cout << "    ";
            it->second->printInfo();  // Здесь используется метод User::printInfo()
        } else {
            std::cout << "    [deleted user " << uid << "]" << std::endl;
        }
    }
    std::cout << "]}" << std::endl;
}

// ============================================================================
// МЕНЕДЖЕР (хранилище) — практически без изменений
// ============================================================================
class UserGroupManager {
private:
    std::unordered_map<int, std::shared_ptr<User>> users;
    std::unordered_map<int, std::shared_ptr<Group>> groups;

public:
    void createUser(int userId, const std::string& username, const std::string& email = "") {
        if (users.find(userId) != users.end()) {
            std::cout << "Error: User " << userId << " already exists!" << std::endl;
            return;
        }
        users[userId] = std::make_shared<User>(userId, username, email);
        std::cout << "User " << userId << " created." << std::endl;
    }

    void deleteUser(int userId) {
        auto it = users.find(userId);
        if (it == users.end()) {
            std::cout << "Error: User " << userId << " not found!" << std::endl;
            return;
        }
        for (auto& [gid, grp] : groups) {
            grp->removeUser(userId);
        }
        users.erase(it);
        std::cout << "User " << userId << " deleted." << std::endl;
    }

    void allUsers() const {
        std::cout << "=== All Users ===" << std::endl;
        for (const auto& [id, user] : users) {
            user->printInfo();
        }
    }

    void getUser(int userId) const {
        auto it = users.find(userId);
        if (it == users.end()) {
            std::cout << "User " << userId << " not found!" << std::endl;
            return;
        }
        it->second->printInfo();
    }

    void createGroup(int groupId) {
        if (groups.find(groupId) != groups.end()) {
            std::cout << "Error: Group " << groupId << " already exists!" << std::endl;
            return;
        }
        groups[groupId] = std::make_shared<Group>(groupId);
        std::cout << "Group " << groupId << " created." << std::endl;
    }

    void deleteGroup(int groupId) {
        auto it = groups.find(groupId);
        if (it == groups.end()) {
            std::cout << "Error: Group " << groupId << " not found!" << std::endl;
            return;
        }
        for (auto& [uid, user] : users) {
            auto userGroup = user->getGroup();
            if (userGroup && userGroup->getId() == groupId) {
                user->setGroup(nullptr);
            }
        }
        groups.erase(it);
        std::cout << "Group " << groupId << " deleted." << std::endl;
    }

    void allGroups() const {
        std::cout << "=== All Groups ===" << std::endl;
        for (const auto& [id, grp] : groups) {
            grp->printInfo(users);
        }
    }

    void getGroup(int groupId) const {
        auto it = groups.find(groupId);
        if (it == groups.end()) {
            std::cout << "Group " << groupId << " not found!" << std::endl;
            return;
        }
        it->second->printInfo(users);
    }

    void addUserToGroup(int userId, int groupId) {
        auto userIt = users.find(userId);
        auto groupIt = groups.find(groupId);

        if (userIt == users.end()) {
            std::cout << "User " << userId << " not found!" << std::endl;
            return;
        }
        if (groupIt == groups.end()) {
            std::cout << "Group " << groupId << " not found!" << std::endl;
            return;
        }

        userIt->second->setGroup(groupIt->second);
        groupIt->second->addUser(userId);
        std::cout << "User " << userId << " added to group " << groupId << std::endl;
    }
};

// ============================================================================
// ТОЧКА ВХОДА
// ============================================================================
int main() {
    UserGroupManager manager;
    std::string command;

    std::cout << "User/Group Management System. Commands:" << std::endl;
    std::cout << "  createUser <id> <name> [email]" << std::endl;
    std::cout << "  deleteUser <id>" << std::endl;
    std::cout << "  allUsers" << std::endl;
    std::cout << "  getUser <id>" << std::endl;
    std::cout << "  createGroup <id>" << std::endl;
    std::cout << "  deleteGroup <id>" << std::endl;
    std::cout << "  allGroups" << std::endl;
    std::cout << "  getGroup <id>" << std::endl;
    std::cout << "  addToGroup <userId> <groupId>" << std::endl;
    std::cout << "  quit" << std::endl;

    while (true) {
        std::cout << "> ";
        std::cin >> command;

        if (command == "createUser") {
            int id; std::string name, email;
            std::cin >> id >> name;
            if (std::cin.peek() != '\n') std::cin >> email;
            manager.createUser(id, name, email);
        }
        else if (command == "deleteUser") {
            int id; std::cin >> id;
            manager.deleteUser(id);
        }
        else if (command == "allUsers") {
            manager.allUsers();
        }
        else if (command == "getUser") {
            int id; std::cin >> id;
            manager.getUser(id);
        }
        else if (command == "createGroup") {
            int id; std::cin >> id;
            manager.createGroup(id);
        }
        else if (command == "deleteGroup") {
            int id; std::cin >> id;
            manager.deleteGroup(id);
        }
        else if (command == "allGroups") {
            manager.allGroups();
        }
        else if (command == "getGroup") {
            int id; std::cin >> id;
            manager.getGroup(id);
        }
        else if (command == "addToGroup") {
            int uid, gid; std::cin >> uid >> gid;
            manager.addUserToGroup(uid, gid);
        }
        else if (command == "quit" || command == "exit") {
            break;
        }
        else {
            std::cout << "Unknown command!" << std::endl;
        }
    }
    return 0;
}
