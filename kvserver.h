#pragma once

#include "kvraft.pb.h"
#include "raft.h"

#include <mutex>

namespace raftkv {

class KvServer {
   public:
    KvServer(int me,int num_of_server);
    ~KvServer();
    void AppendEntries(const kvraft::AppendEntriesArgs &req,
                      kvraft::AppendEntriesReply *resp);

    int RequestVote(const kvraft::RequestVoteArgs &req,
                    kvraft::RequestVoteReply *resp);
    int StartCommand(kvraft::Operation &op);
    int Command(const kvraft::KVArgs &req, kvraft::KVReply *resp);

   private:
    std::mutex kvserver_mutex_;
    Raft raft_;
};

}  // namespace raftkv