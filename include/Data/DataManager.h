#pragma once
#include <string>
#include <vector>

class DataManager {
public:
    static bool SaveLibraryData(const std::string& filepath, 
                               const std::vector<std::string>& paths,
                               const std::vector<std::string>& titles);
    static bool LoadLibraryData(const std::string& filepath,
                               std::vector<std::string>& paths,
                               std::vector<std::string>& titles);
    
    static bool SaveTodoList(const std::string& filepath,
                            const std::vector<std::string>& tasks,
                            const std::vector<bool>& completed);
    static bool LoadTodoList(const std::string& filepath,
                            std::vector<std::string>& tasks,
                            std::vector<bool>& completed);
    
private:
    static std::string EscapeJson(const std::string& str);
    static std::string UnescapeJson(const std::string& str);
};
