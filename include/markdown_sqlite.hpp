#pragma once
#include <sqlite3.h>
#include <markdown_parser.hpp>

namespace MSQLite{
class Database {
public:
    Database(const char* path);
    void create_table();
    short safe_create_table() noexcept;
    void insert_document(const std::vector<MParser::Header> header,
                         const std::string raw_content,
                         const std::unordered_set<std::string> keywords) ;
    void insert_keyword(int id, std::string keyword);
    std::vector<std::pair<std::string, std::string>> search_keyword(std::string keyword) ;
    int test_database();

private:
    sqlite3* db_;

    template<typename T>
    std::string my_to_string(T val) {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }
};
}
