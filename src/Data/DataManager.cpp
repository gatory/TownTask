#include "Data/DataManager.h"
#include <fstream>
#include <sstream>

std::string DataManager::EscapeJson(const std::string& str) {
    std::string result;
    for (char c : str) {
        if (c == '"') result += "\\\"";
        else if (c == '\\') result += "\\\\";
        else if (c == '\n') result += "\\n";
        else if (c == '\r') result += "\\r";
        else if (c == '\t') result += "\\t";
        else result += c;
    }
    return result;
}

std::string DataManager::UnescapeJson(const std::string& str) {
    std::string result;
    bool escape = false;
    for (char c : str) {
        if (escape) {
            if (c == 'n') result += '\n';
            else if (c == 'r') result += '\r';
            else if (c == 't') result += '\t';
            else result += c;
            escape = false;
        } else if (c == '\\') {
            escape = true;
        } else {
            result += c;
        }
    }
    return result;
}

bool DataManager::SaveLibraryData(const std::string& filepath,
                                  const std::vector<std::string>& paths,
                                  const std::vector<std::string>& titles) {
    std::ofstream file(filepath);
    if (!file.is_open()) return false;
    
    file << "{\n  \"ebooks\": [\n";
    for (size_t i = 0; i < paths.size(); i++) {
        file << "    {\n";
        file << "      \"path\": \"" << EscapeJson(paths[i]) << "\",\n";
        file << "      \"title\": \"" << EscapeJson(titles[i]) << "\"\n";
        file << "    }";
        if (i < paths.size() - 1) file << ",";
        file << "\n";
    }
    file << "  ]\n}\n";
    
    file.close();
    return true;
}

bool DataManager::LoadLibraryData(const std::string& filepath,
                                  std::vector<std::string>& paths,
                                  std::vector<std::string>& titles) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;
    
    paths.clear();
    titles.clear();
    
    std::string line;
    std::string currentPath, currentTitle;
    
    while (std::getline(file, line)) {
        // Simple JSON parsing (not robust, but works for this format)
        size_t pathPos = line.find("\"path\": \"");
        if (pathPos != std::string::npos) {
            size_t start = pathPos + 9;
            size_t end = line.find("\"", start);
            currentPath = UnescapeJson(line.substr(start, end - start));
        }
        
        size_t titlePos = line.find("\"title\": \"");
        if (titlePos != std::string::npos) {
            size_t start = titlePos + 10;
            size_t end = line.find("\"", start);
            currentTitle = UnescapeJson(line.substr(start, end - start));
            
            paths.push_back(currentPath);
            titles.push_back(currentTitle);
        }
    }
    
    file.close();
    return true;
}

bool DataManager::SaveTodoList(const std::string& filepath,
                               const std::vector<std::string>& tasks,
                               const std::vector<bool>& completed) {
    std::ofstream file(filepath);
    if (!file.is_open()) return false;
    
    file << "{\n  \"tasks\": [\n";
    for (size_t i = 0; i < tasks.size(); i++) {
        file << "    {\n";
        file << "      \"task\": \"" << EscapeJson(tasks[i]) << "\",\n";
        file << "      \"completed\": " << (completed[i] ? "true" : "false") << "\n";
        file << "    }";
        if (i < tasks.size() - 1) file << ",";
        file << "\n";
    }
    file << "  ]\n}\n";
    
    file.close();
    return true;
}

bool DataManager::LoadTodoList(const std::string& filepath,
                               std::vector<std::string>& tasks,
                               std::vector<bool>& completed) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;
    
    tasks.clear();
    completed.clear();
    
    std::string line;
    std::string currentTask;
    bool currentCompleted = false;
    
    while (std::getline(file, line)) {
        size_t taskPos = line.find("\"task\": \"");
        if (taskPos != std::string::npos) {
            size_t start = taskPos + 9;
            size_t end = line.find("\"", start);
            currentTask = UnescapeJson(line.substr(start, end - start));
        }
        
        size_t completedPos = line.find("\"completed\": ");
        if (completedPos != std::string::npos) {
            currentCompleted = line.find("true", completedPos) != std::string::npos;
            
            tasks.push_back(currentTask);
            completed.push_back(currentCompleted);
        }
    }
    
    file.close();
    return true;
}