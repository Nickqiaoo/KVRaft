#include "snapshot.h"
#include "raft.h"

namespace raftkv {

void Snapshot::SaveSnapshot() {

    for (auto it = raft_->log_.begin(); it != raft_->log_.end(); it++) {
        if (it->op == LogEntry::PUT) {
            snapshot_->Put(it->key, it->value);
        } else {
            snapshot_->Delete(it->key);
        }
    }
}

}  // namespace raftkv