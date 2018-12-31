#pragma once

#include "kvraft.pb.h"
#include "raft.h"

#include <mutex>

namespace kvraft {

class KVServer {
   public:
    int AppendEntries(const kvraft::AppendEntriesArgs &req,
                      kvraft::AppendEntriesReply *resp);

    int RequestVote(const kvraft::RequestVoteArgs &req,
                    kvraft::RequestVoteReply *resp);
    int StartCommand(kvraft::Operation &op);
    int Command(const kvraft::KVArgs &req, kvraft::KVReply *resp);

   private:
    std::mutex kvraft_mutex_;
    Raft raft_;
};

}  // namespace kvraft