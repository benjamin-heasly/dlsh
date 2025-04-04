#ifndef CG_TAB_MANAGER_H
#define CG_TAB_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>

// Forward declaration of CGWin class
class CGWin;

class CGTabManager {
public:
    // Get the singleton instance
    static CGTabManager& getInstance();
    
    // Add a new CGWin widget with the given name
    void addCGWin(const std::string& name, CGWin* widget);
    
    // Get a CGWin widget by name
    CGWin* getCGWin(const std::string& name);
    
    // Remove a CGWin widget by name
    void removeCGWin(const std::string& name);
    
    // Get the next available tab name in the format "cg0", "cg1", etc.
    std::string getNextTabName();

private:
    // Private constructor for singleton
    CGTabManager() = default;
    
    // Delete copy constructor and assignment operator
    CGTabManager(const CGTabManager&) = delete;
    CGTabManager& operator=(const CGTabManager&) = delete;
    
    // Map to store CGWin widgets
    std::unordered_map<std::string, CGWin*> cgwin_map;
    
    // Counter for generating tab names
    int next_tab_index = 0;
};

#endif // CG_TAB_MANAGER_H 