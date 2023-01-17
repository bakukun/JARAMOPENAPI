//bytetest.cpp
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <string>
#include <leveldb/db.h>
using namespace std;

int main(void)
{
    leveldb::DB *db = nullptr;
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, "./testdb", &db);
    assert(status.ok());
    cout<<sizeof(size_t)<<endl;
    std::string key = "A";
    std::string value = string(4300000000,'h');
    std::string get_value;
    leveldb::Status s = db->Put(leveldb::WriteOptions(), key, value);
    if (s.ok())
        s = db->Get(leveldb::ReadOptions(), "A", &get_value);
    if (s.ok())
        std::cout << get_value.size() << std::endl;
    else
        std::cout << s.ToString() << std::endl;
    delete db;
    return 0;
}
