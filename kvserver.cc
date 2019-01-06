#include "kvserver.h"
#include "raft.h"

#include <mutex>

namespace raftkv {

using namespace phxrpc;

KvServer::KvServer(int me,int num_of_server) : raft_(me,num_of_server) {}
KvServer::~KvServer() {}

}  // namespace raftkv