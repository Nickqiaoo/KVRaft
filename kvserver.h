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
    void AppendEntries(const kvraft::AppendEntriesArgs& req, kvraft::AppendEntriesReply* resp);

    void RequestVote(const kvraft::RequestVoteArgs& req, kvraft::RequestVoteReply* resp);
    void PutDelete(const string& key, const string& value);
    void Command(const kvraft::KVArgs& req, kvraft::KVReply* resp,UThreadEpollScheduler *scheduler);
    void StartCommand(const kvraft::Operation& operation,kvraft::KVReply *resp,UThreadEpollScheduler* scheduler);

   private:
    std::mutex kvserver_mutex_;
    Raft raft_;
    std::map<int, int> clientseq_;
    Storage* storage_;
};

}  // namespace raftkv