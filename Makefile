all:
	mkdir -p obj
	g++ src/markdown_sqlite.cpp -std=c++23 -I../MarkdownParser/include -L../MarkdownParser/lib -lMdParser -lsqlite3 -Iinclude -c -o obj/markdown_sqlite.o 
	mkdir -p lib 
	ar rcs lib/libMdSQLite.a obj/markdown_sqlite.o 
test: 
	g++ src/markdown_sqlite_test.cpp -std=c++23 -I../MarkdownParser/include/ -Iinclude -c -o obj/markdown_sqlite_test.o 
	mkdir -p target
	g++ obj/markdown_sqlite_test.o -o target/test -Llib -lMdSQLite -lsqlite3 -L../MarkdownParser/lib/ -lMdParser
