#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>

// ---------- Классы без циклических зависимостей ----------
class User {
public:
    User(int id, std::string name, std::string additional)
        : id_(id), name_(std::move(name)), additional_(std::move(additional)), groupId_(-1) {}

    int getId() const { return id_; }
    const std::string& getName() const { return name_; }
    const std::string& getAdditional() const { return additional_; }
    int getGroupId() const { return groupId_; }

    void setGroupId(int gid) { groupId_ = gid; }

private:
    int id_;
    std::string name_;
    std::string additional_;
    int groupId_;   // -1 означает "не состоит в группе"
};

class Group {
public:
    explicit Group(int id) : id_(id) {}

    int getId() const { return id_; }
    const std::vector<int>& getUserIds() const { return userIds_; }

    void addUser(int uid) {
        if (std::find(userIds_.begin(), userIds_.end(), uid) == userIds_.end())
            userIds_.push_back(uid);
    }

    void removeUser(int uid) {
        auto it = std::find(userIds_.begin(), userIds_.end(), uid);
        if (it != userIds_.end())
            userIds_.erase(it);
    }

private:
    int id_;
    std::vector<int> userIds_;
};

// ---------- Глобальное хранилище (можно обернуть в класс, но для простоты -- так) ----------
std::map<int, User> users;
std::map<int, Group> groups;

// ---------- Вспомогательные функции вывода ----------
void printUser(const User& u) {
    std::cout << "User: id=" << u.getId()
              << ", name=" << u.getName()
              << ", additional=" << u.getAdditional();
    if (u.getGroupId() != -1)
        std::cout << ", groupId=" << u.getGroupId();
    else
        std::cout << ", group=none";
    std::cout << std::endl;
}

void printGroup(const Group& g) {
    std::cout << "Group: id=" << g.getId() << ", members: ";
    const auto& memberIds = g.getUserIds();
    if (memberIds.empty()) {
        std::cout << "none";
    } else {
        for (size_t i = 0; i < memberIds.size(); ++i) {
            int uid = memberIds[i];
            auto it = users.find(uid);
            std::cout << uid << "(" << (it != users.end() ? it->second.getName() : "?") << ")";
            if (i + 1 < memberIds.size()) std::cout << ", ";
        }
    }
    std::cout << std::endl;
}

// ---------- Команды ----------
void createUser(int id, const std::string& name, const std::string& additional) {
    if (users.count(id)) {
        std::cerr << "Error: user with id " << id << " already exists" << std::endl;
        return;
    }
    users.emplace(id, User(id, name, additional));
    std::cout << "User created." << std::endl;
}

void deleteUser(int id) {
    auto it = users.find(id);
    if (it == users.end()) {
        std::cerr << "Error: user " << id << " not found" << std::endl;
        return;
    }
    int groupId = it->second.getGroupId();
    if (groupId != -1) {
        auto git = groups.find(groupId);
        if (git != groups.end())
            git->second.removeUser(id);
    }
    users.erase(it);
    std::cout << "User deleted." << std::endl;
}

void allUsers() {
    if (users.empty()) {
        std::cout << "No users." << std::endl;
        return;
    }
    for (const auto& [id, user] : users)
        printUser(user);
}

void getUser(int id) {
    auto it = users.find(id);
    if (it == users.end()) {
        std::cerr << "Error: user " << id << " not found" << std::endl;
        return;
    }
    printUser(it->second);
}

void createGroup(int id) {
    if (groups.count(id)) {
        std::cerr << "Error: group with id " << id << " already exists" << std::endl;
        return;
    }
    groups.emplace(id, Group(id));
    std::cout << "Group created." << std::endl;
}

void deleteGroup(int id) {
    auto it = groups.find(id);
    if (it == groups.end()) {
        std::cerr << "Error: group " << id << " not found" << std::endl;
        return;
    }
    // У всех пользователей, состоявших в этой группе, сбрасываем groupId
    for (int uid : it->second.getUserIds()) {
        auto uit = users.find(uid);
        if (uit != users.end())
            uit->second.setGroupId(-1);
    }
    groups.erase(it);
    std::cout << "Group deleted." << std::endl;
}

void allGroups() {
    if (groups.empty()) {
        std::cout << "No groups." << std::endl;
        return;
    }
    for (const auto& [id, group] : groups)
        printGroup(group);
}

void getGroup(int id) {
    auto it = groups.find(id);
    if (it == groups.end()) {
        std::cerr << "Error: group " << id << " not found" << std::endl;
        return;
    }
    printGroup(it->second);
}

// ---------- Главный цикл обработки команд ----------
int main() {
    std::string line;
    std::cout << "User/Group Manager. Commands:\n"
                 "  createUser <id> <name> [additional...]\n"
                 "  deleteUser <id>\n"
                 "  allUsers\n"
                 "  getUser <id>\n"
                 "  createGroup <id>\n"
                 "  deleteGroup <id>\n"
                 "  allGroups\n"
                 "  getGroup <id>\n"
                 "  exit\n";
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line))
            break;
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "exit") break;

        else if (cmd == "createUser") {
            int id; std::string name, additional;
            if (!(iss >> id >> name)) {
                std::cerr << "Usage: createUser <id> <name> [additional...]" << std::endl;
                continue;
            }
            std::getline(iss, additional);
            // убираем ведущий пробел
            if (!additional.empty() && additional[0] == ' ')
                additional.erase(0, 1);
            createUser(id, name, additional);
        }
        else if (cmd == "deleteUser") {
            int id;
            if (!(iss >> id)) {
                std::cerr << "Usage: deleteUser <id>" << std::endl;
                continue;
            }
            deleteUser(id);
        }
        else if (cmd == "allUsers") {
            allUsers();
        }
        else if (cmd == "getUser") {
            int id;
            if (!(iss >> id)) {
                std::cerr << "Usage: getUser <id>" << std::endl;
                continue;
            }
            getUser(id);
        }
        else if (cmd == "createGroup") {
            int id;
            if (!(iss >> id)) {
                std::cerr << "Usage: createGroup <id>" << std::endl;
                continue;
            }
            createGroup(id);
        }
        else if (cmd == "deleteGroup") {
            int id;
            if (!(iss >> id)) {
                std::cerr << "Usage: deleteGroup <id>" << std::endl;
                continue;
            }
            deleteGroup(id);
        }
        else if (cmd == "allGroups") {
            allGroups();
        }
        else if (cmd == "getGroup") {
            int id;
            if (!(iss >> id)) {
                std::cerr << "Usage: getGroup <id>" << std::endl;
                continue;
            }
            getGroup(id);
        }
        else {
            std::cerr << "Unknown command: " << cmd << std::endl;
        }
    }
    return 0;
}
