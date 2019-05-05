#include "leveldb_storage.h"

namespace raftkv {

bool LevelDB::Get(const string& key, string* value) {
    leveldb::Status s = db_->Get(leveldb::ReadOptions(), key, value);
    return s.ok();
}
void LevelDB::Put(const string& key, const string& value) {
    db_->Put(leveldb::WriteOptions(), key, value);
}
void LevelDB::Delete(const string& key) { db_->Delete(leveldb::WriteOptions(), key); }

}  // namespace raftkv