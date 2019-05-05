#include "kvserver.h"
#include "leveldb_storage.h"
#include "raft.h"

#include <sys/eventfd.h>
#include <mutex>

namespace raftkv {

using namespace phxrpc;

KvServer::KvServer(int me, int num_of_server)
    : raft_(me, num_of_server), storage_(new LevelDB("test")) {}
KvServer::~KvServer() { delete storage_; }

void KvServer::AppendEntries(const kvraft::AppendEntriesArgs &req,
                             kvraft::AppendEntriesReply *resp) {
    raft_.AppendEntries(req, resp);
}
void KvServer::RequestVote(const kvraft::RequestVoteArgs &req, kvraft::RequestVoteReply *resp) {
    raft_.RequestVote(req, resp);
}

void KvServer::Command(const kvraft::KVArgs &req, kvraft::KVReply *resp,
                       UThreadEpollScheduler *scheduler) {
    //printf("%s\n", __func__);
    kvraft::Operation op = req.command();
    auto it = clientseq_.find(req.clientid());
    if (it == clientseq_.end()) {
        clientseq_[req.clientid()] = req.seq();
        StartCommand(req.command(), resp, scheduler);
    } else if (it->second < req.seq()) {
        StartCommand(req.command(), resp, scheduler);
    } else {
        //printf("timeout\n");
        resp->set_res(kvraft::KVResult::ErrTimeout);
    }
}

void KvServer::StartCommand(const kvraft::Operation &operation, kvraft::KVReply *resp,
                            UThreadEpollScheduler *scheduler) {
    //printf("%s\n", __func__);
    if (operation.op() == kvraft::Operation_OpName_GET) {
        string value;
        if (storage_->Get(operation.key(), &value)) {
            cout<<"GET "<<operation.key()<<endl;
            resp->set_res(kvraft::OK);
            resp->set_value(value);
        } else {
            resp->set_res(kvraft::ErrNoKey);
        }
    } else {
        int evfd = eventfd(0, EFD_NONBLOCK);
        UThreadSocket_t *socket = scheduler->CreateSocket(evfd);
        pair<int, bool> res = raft_.Start((raftkv::LogEntry::operation)operation.op(),
                                          operation.key(), operation.value(), evfd);
        if (res.second == false) {
            resp->set_res(kvraft::ErrWrongLeader);
        } else {
            uint64_t buf;
            int ret = UThreadRead(*socket, (void *)&buf, sizeof(uint64_t), 0);
            if (ret > 0) {
                if (operation.op() == kvraft::Operation_OpName_PUT) {
                    cout<<"PUT "<<operation.key()<<" "<<operation.value()<<endl;
                    storage_->Put(operation.key(), operation.value());
                } else {
                    cout<<"DEL "<<operation.key()<<endl;
                    storage_->Delete(operation.key());
                }
                resp->set_res(kvraft::OK);
            } else {
                printf("timeout\n");
                resp->set_res(kvraft::ErrTimeout);
            }
            free(socket);
            close(evfd);
        }
    }
}

}  // namespace raftkv