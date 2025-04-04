#include "cg_tab_manager.h"
#include <sstream>

CGTabManager& CGTabManager::getInstance() {
    static CGTabManager instance;
    return instance;
}

void CGTabManager::addCGWin(const std::string& name, CGWin* widget) {
    cgwin_map[name] = widget;
}

CGWin* CGTabManager::getCGWin(const std::string& name) {
    auto it = cgwin_map.find(name);
    if (it != cgwin_map.end()) {
        return it->second;
    }
    return nullptr;
}

void CGTabManager::removeCGWin(const std::string& name) {
    cgwin_map.erase(name);
}

std::string CGTabManager::getNextTabName() {
    std::stringstream ss;
    ss << "cg" << next_tab_index++;
    return ss.str();
} 