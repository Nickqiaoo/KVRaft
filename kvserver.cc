#include "kvserver.h"
#include "raft.h"

#include <mutex>

namespace raftkv {

using namespace phxrpc;

KvServer::KvServer(int me) : raft_(me) {}
KvServer::~KvServer() {}

}  // namespace raftkv