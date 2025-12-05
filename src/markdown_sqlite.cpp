#include "markdown_sqlite.hpp"

namespace MSQLite {
Database::Database(const char* path) {
    if (sqlite3_open(path, &db_) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db_));
    }
    // 启用WAL模式提升并发性能
    // execute("PRAGMA journal_mode=WAL;");
}
Database::~Database() {
    sqlite3_close(db_);
}
void Database::create_table() {
    const char* sql =
        " CREATE TABLE IF NOT EXISTS documents ( id INTEGER PRIMARY KEY, title TEXT NOT NULL, raw_text TEXT, last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP); CREATE TABLE IF NOT EXISTS keywords ( id INTEGER PRIMARY KEY, doc_id INTEGER , keyword TEXT NOT NULL, UNIQUE(doc_id, keyword)); ";
    char* errmsg = NULL;
    if (
        sqlite3_exec(db_, sql, 0, 0, &errmsg) != SQLITE_OK
    ) {
        throw std::runtime_error("创建SQL表失败");
    }
}
short Database::safe_create_table() noexcept {
    try {
        create_table();
        return 0;
    } catch(...) {
        return 1;
    }
}
void Database::insert_document(const std::vector<MParser::Header> header,
                               const std::string raw_content,
                               const std::unordered_set<std::string> keywords) {
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO documents(title, raw_text) VALUES(?, ?)";
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

    // 绑定标题（取第一个H1标题）
    std::string title = header.empty() ? "Untitled" : header[0].text;
    sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_TRANSIENT);

    // 绑定原始内容
    sqlite3_bind_text(stmt, 2, raw_content.c_str(), -1, SQLITE_TRANSIENT);

    sqlite3_step(stmt);
    int doc_id = sqlite3_last_insert_rowid(db_);
    sqlite3_finalize(stmt);

    // 插入关键词
    for (const auto& kw : keywords) {
        insert_keyword(doc_id, kw);
    }
}
void Database::insert_keyword(int id, std::string keyword) {
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO keywords(doc_id, keyword) VALUES(?, ?)";
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

    sqlite3_bind_text(stmt, 1, std::to_string(id).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, keyword.c_str(), -1, SQLITE_TRANSIENT);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}
std::vector<std::pair<std::string, std::string>> Database::search_keyword(std::string keyword) {
    sqlite3_stmt* stmt, *stmt_inwhile;
    const char* sql = "SELECT id, doc_id FROM keywords WHERE keyword LIKE ?";
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

    std::string search = "%" + keyword + "%";
    sqlite3_bind_text(stmt, 1, search.c_str(), -1, SQLITE_TRANSIENT);

    int rc;
    std::vector<std::pair<std::string, std::string>> vecret;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        int doc_id = sqlite3_column_int(stmt, 1);

        const char* sql = "SELECT id, title, raw_text FROM documents WHERE id = ?";
        sqlite3_prepare_v2(db_, sql, -1, &stmt_inwhile, nullptr);

        sqlite3_bind_text(stmt_inwhile, 1, std::to_string(doc_id).c_str(), -1, SQLITE_TRANSIENT);

        sqlite3_step(stmt_inwhile);
        std::string title =
            my_to_string(sqlite3_column_text(stmt_inwhile, 1));
        std::string raw_text = my_to_string(sqlite3_column_text(stmt_inwhile, 2));
        sqlite3_finalize(stmt_inwhile);

        vecret.push_back(std::make_pair(title, raw_text));
    }
    sqlite3_finalize(stmt);
    return vecret;
}
int Database::test_database() {
    std::string Markdown = R"(
# 某些事情
## 二级标题
**关键词**
`代码`
### 三级标题
## 又一个二级标题
    )";
    MParser::MarkdownParser p;
    p.parse(Markdown);
    auto [a, b] = p.get_results();
    Database testdb("test.db");
    testdb.create_table();
    testdb.insert_document(a, Markdown, b);
    auto returnval = testdb.search_keyword("关键词");
    for(const auto& i: returnval) {
        std::cout << "标题: " << i.first << std::endl;
    }
    return 0;
}
}
