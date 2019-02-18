#pragma once

#include "kvraft.pb.h"
#include "raft.h"
#include "storage.h"

#include <mutex>

namespace raftkv {

class KvServer {
   public:
    KvServer(int me, int num_of_server);
    ~KvServer();
    void AppendEntries(const kvraft::AppendEntriesArgs &req, kvraft::AppendEntriesReply *resp);

    void RequestVote(const kvraft::RequestVoteArgs &req, kvraft::RequestVoteReply *resp);
    void StartCommand(kvraft::Operation &op);
    void Command(const kvraft::KVArgs &req, kvraft::KVReply *resp);

   private:
    std::mutex kvserver_mutex_;
    Raft raft_;
    std::map<int, int> channel_;
    Storage* storage_;
};

}  // namespace raftkv