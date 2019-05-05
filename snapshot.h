#pragma once

#include "leveldb_storage.h"
#include "storage.h"

namespace raftkv {

class Raft;

class Snapshot {
   public:
    Snapshot(Raft* raft) : snapshot_(new LevelDB("snapshot")), raft_(raft) {}
    ~Snapshot() { delete snapshot_; }
    void SaveSnapshot();

   private:
    Storage* snapshot_;
    Raft* raft_;
};
}  // namespace raftkv