#include "kvserver.h"
#include "raft.h"

#include <mutex>

namespace raftkv {

using namespace phxrpc;

KvServer::KvServer(int me, int num_of_server) : raft_(me, num_of_server) {}
KvServer::~KvServer() {}

void KvServer::AppendEntries(const kvraft::AppendEntriesArgs &req,
                             kvraft::AppendEntriesReply *resp) {
    raft_.AppendEntries(req, resp);
}
void KvServer::RequestVote(const kvraft::RequestVoteArgs &req, kvraft::RequestVoteReply *resp){
    raft_.RequestVote(req,resp);
}

}  // namespace raftkv