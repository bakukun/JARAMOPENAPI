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

// loads multiple key-value pair written in the text file
void addKeyValues(leveldb::DB *db, std::ifstream& file) {
    if (file.is_open()) {
        leveldb::Status s;
        string line;
        string key;
        string value;
        srand((unsigned int)time(NULL));
        int tmp = rand();
                                                              1,1        꼭대기
