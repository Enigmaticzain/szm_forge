#include <iostream>
#include <fstream>
#include <string>

int main() {
    std::ifstream f("src/WebAPI/FastAPIBridge.cpp");
    if (!f) return 1;
    std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    int depth = 0;
    bool in_str = false;
    bool in_raw_str = false;
    bool in_char = false;
    bool in_line_comment = false;
    bool in_block_comment = false;
    
    for (size_t i = 0; i < content.size(); ++i) {
        if (in_line_comment) {
            if (content[i] == '\n') in_line_comment = false;
            continue;
        }
        if (in_block_comment) {
            if (content[i] == '*' && i + 1 < content.size() && content[i+1] == '/') {
                in_block_comment = false;
                i++;
            }
            continue;
        }
        if (in_raw_str) {
            if (content[i] == ')' && i + 1 < content.size() && content[i+1] == '"') {
                in_raw_str = false;
                i++;
            }
            continue;
        }
        if (in_str) {
            if (content[i] == '\\') { i++; continue; }
            if (content[i] == '"') in_str = false;
            continue;
        }
        if (in_char) {
            if (content[i] == '\\') { i++; continue; }
            if (content[i] == '\'') in_char = false;
            continue;
        }
        
        if (content[i] == '/' && i + 1 < content.size() && content[i+1] == '/') {
            in_line_comment = true; i++; continue;
        }
        if (content[i] == '/' && i + 1 < content.size() && content[i+1] == '*') {
            in_block_comment = true; i++; continue;
        }
        if (content[i] == 'R' && i + 2 < content.size() && content[i+1] == '"' && content[i+2] == '(') {
            in_raw_str = true; i += 2; continue;
        }
        if (content[i] == '"') { in_str = true; continue; }
        if (content[i] == '\'') { in_char = true; continue; }
        
        if (content[i] == '{') { depth++; }
        else if (content[i] == '}') {
            depth--;
            if (depth == 0) {
                int line = 1;
                for (size_t j = 0; j <= i; ++j) if (content[j] == '\n') line++;
                std::cout << "Depth hit 0 at line " << line << "\n";
            }
        }
    }
    std::cout << "Final depth: " << depth << "\n";
    return 0;
}
