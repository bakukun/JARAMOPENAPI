// leveldb_functions_test.cpp

#include <iostream>
#include <fstream>
#include <cassert>
#include <sstream>
#include <vector>
#include <string>
#include "leveldb/db.h"
#include "leveldb/write_batch.h"

using namespace std;

//split function
vector<string> split(string str, char Delimiter) {
    istringstream iss(str);
    string buffer;

    vector<string> result;

    while (getline(iss, buffer, Delimiter)) {
        result.push_back(buffer);
    }

    return result;
}

// loads multiple key-value pair written in the text file
void addKeyValues(leveldb::DB *db, std::ifstream& file) {
    if (file.is_open()) {
        leveldb::Status s;
        string line;
        string key;
        string value;

        while (getline(file, line)) {
            vector<string> result = split(line, ' ');
            key = result[0];
            value = result[1];

            s=db->Put(leveldb::WriteOptions(), key, value);
        }
    } else {
        cout << "File not opened" << endl;
        return;
    }
    return;
}

// iterate through all key-value pairs and print them out
void printAllKVPairs(leveldb::DB * db) {
    leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());

    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        cout << it->key().ToString() << ": " << it->value().ToString() << endl;
    }
    assert(it->status().ok());

    delete it;
    return;
}

int main(int argc, char *argv[]) {
    leveldb::DB* db;
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, "/tmp/testdb", &db);
    assert(status.ok());

    leveldb::Status s;

    string filename = argv[1];
    std::ifstream file(filename);

    addKeyValues(db, file);  //custom function to load DB with key-value pairs
    printAllKVPairs(db);  // custom function to print all key-value pairs
    delete db;

    return 0;
}
