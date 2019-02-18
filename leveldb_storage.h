#pragma once

#include <leveldb/db.h>

#include "storage.h"

namespace raftkv {

class LevelDB : public Storage {
   public:
    LevelDB() {
        options_.create_if_missing = true;
        leveldb::Status status = leveldb::DB::Open(options_, "/tmp/testdb", &db_);
        assert(status.ok());
    }
    ~LevelDB() { delete db_; }

    virtual bool Get(const string& key, string* value) override;
    virtual void Put(const string& key,const string& value) override;
    virtual void Delete(const string& key) override;

   private:
    leveldb::DB* db_;
    leveldb::Options options_;
};

}  // namespace raftkv