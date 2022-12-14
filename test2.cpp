#include <iostream>
#include <fstream>
#include <cassert>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include "leveldb/db.h"
#include "leveldb/write_batch.h"

using namespace std;
leveldb::Slice a;
leveldb::Slice b;

static int Compare(const leveldb::Slice &a, const leveldb::Slice &b) {
    const size_t min_len = (a.size_ < b.size_) ? a.size : b.size_;
    int r = memcmp(a.data_,b.data_,min_len);
    if (r==0) {
       if (a.size_ < b.size_) r = -1;
       else if (a.size_ > b.size) r = +1;
	}
	return r;
    }


// loads multiple key-value pair written in the text file
void addKeyValues(leveldb::DB *db, std::ifstream& file) {
    if (file.is_open()) {
        leveldb::Status s;
        string line;
        string key;
	string value;
	srand((unsigned int)time(NULL));
	int tmp = rand();
	tmp = (int)tmp % 10;
	//0 < value <10 
	if (tmp == 0) {
		tmp = 1;
	}


        while (getline(file, line)) {
            key = line;
            value = to_string(tmp);
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
    leveldb::Status s1;
    leveldb::Status s2;
 
    string filename = argv[1];
    std::ifstream file(filename);

    addKeyValues(db, file);  //custom function to load DB with key-value pairs
    printAllKVPairs(db);  // custom function to print all key-value pairs
    
    delete db;

    return 0;
}
