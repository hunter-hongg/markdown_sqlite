#include <markdown_sqlite.hpp>

int main() {
    MSQLite::Database db("");
    db.test_database();
    return 0;
}
