/* kvraft_client_uthread.cpp

 Generated by phxrpc_pb2client from kvraft.proto

*/

#include "kvraft_client_uthread.h"

#include <cstdlib>
#include <memory>
#include <mutex>

#include "phxrpc/http.h"
#include "phxrpc/rpc.h"

#include "phxrpc_kvraft_stub.h"


using namespace std;


static phxrpc::ClientConfig global_kvraftclientuthread_config_;
static phxrpc::ClientMonitorPtr global_kvraftclientuthread_monitor_;


bool KVRaftClientUThread::Init(const char *config_file) {
    return global_kvraftclientuthread_config_.Read(config_file);
}

const char *KVRaftClientUThread::GetPackageName() {
    const char *ret{global_kvraftclientuthread_config_.GetPackageName()};
    if (strlen(ret) == 0) {
        ret = "kvraft";
    }
    return ret;
}

KVRaftClientUThread::KVRaftClientUThread(phxrpc::UThreadEpollScheduler *uthread_scheduler) {
    uthread_scheduler_ = uthread_scheduler;
    static mutex monitor_mutex;
    if (!global_kvraftclientuthread_monitor_.get()) {
        monitor_mutex.lock();
        if (!global_kvraftclientuthread_monitor_.get()) {
            global_kvraftclientuthread_monitor_ = phxrpc::MonitorFactory::GetFactory()->
                    CreateClientMonitor(GetPackageName());
        }
        global_kvraftclientuthread_config_.SetClientMonitor(global_kvraftclientuthread_monitor_);
        monitor_mutex.unlock();
    }
}

KVRaftClientUThread::~KVRaftClientUThread() {
}

int KVRaftClientUThread::PHXEcho(const google::protobuf::StringValue &req, google::protobuf::StringValue *resp)
{
    const phxrpc::Endpoint_t *ep{global_kvraftclientuthread_config_.GetRandom()};

    if (uthread_scheduler_ && ep) {
        phxrpc::UThreadTcpStream socket;
        bool open_ret{phxrpc::PhxrpcTcpUtils::Open(uthread_scheduler_, &socket, ep->ip, ep->port,
                global_kvraftclientuthread_config_.GetConnectTimeoutMS(),
                *(global_kvraftclientuthread_monitor_.get()))};
        if (open_ret) {
            socket.SetTimeout(global_kvraftclientuthread_config_.GetSocketTimeoutMS());
            phxrpc::HttpMessageHandlerFactory http_msg_factory;
            KVRaftStub stub(socket, *(global_kvraftclientuthread_monitor_.get()), http_msg_factory);
            return stub.PHXEcho(req, resp);
        }
    }

    return -1;
}

int KVRaftClientUThread::PHXBatchEcho(const google::protobuf::StringValue &req, google::protobuf::StringValue *resp)
{
    int ret{-1};
    size_t echo_server_count{2};
    uthread_begin;
    for (size_t i{0}; echo_server_count > i; ++i) {
        uthread_t [=, &uthread_s, &ret](void *) {
            const phxrpc::Endpoint_t *ep = global_kvraftclientuthread_config_.GetByIndex(i);
            if (ep != nullptr) {
                phxrpc::UThreadTcpStream socket;
                if (phxrpc::PhxrpcTcpUtils::Open(&uthread_s, &socket, ep->ip, ep->port,
                    global_kvraftclientuthread_config_.GetConnectTimeoutMS(), *(global_kvraftclientuthread_monitor_.get()))) {
                    socket.SetTimeout(global_kvraftclientuthread_config_.GetSocketTimeoutMS());
                    phxrpc::HttpMessageHandlerFactory http_msg_factory;
                    KVRaftStub stub(socket, *(global_kvraftclientuthread_monitor_.get()), http_msg_factory);
                    int this_ret{stub.PHXEcho(req, resp)};
                    if (this_ret == 0) {
                        ret = this_ret;
                        uthread_s.Close();
                    }
                }
            }
        };
    }
    uthread_end;
    return ret;
}

int KVRaftClientUThread::RequestVote(const kvraft::RequestVoteArgs &req, kvraft::RequestVoteReply *resp,int index)
{
    const phxrpc::Endpoint_t *ep{global_kvraftclientuthread_config_.GetByIndex(index)};
    //printf("port:%d\n", ep->port);
    if (uthread_scheduler_ && ep) {
        phxrpc::UThreadTcpStream socket;
        bool open_ret{phxrpc::PhxrpcTcpUtils::Open(uthread_scheduler_, &socket, ep->ip, ep->port,
                global_kvraftclientuthread_config_.GetConnectTimeoutMS(),
                *(global_kvraftclientuthread_monitor_.get()))};
        if (open_ret) {
            socket.SetTimeout(global_kvraftclientuthread_config_.GetSocketTimeoutMS());
            phxrpc::HttpMessageHandlerFactory http_msg_factory;
            KVRaftStub stub(socket, *(global_kvraftclientuthread_monitor_.get()), http_msg_factory);
            return stub.RequestVote(req, resp);
        }
    }

    return -1;
}

int KVRaftClientUThread::AppendEntries(const kvraft::AppendEntriesArgs &req, kvraft::AppendEntriesReply *resp,int index)
{
    const phxrpc::Endpoint_t *ep{global_kvraftclientuthread_config_.GetByIndex(index)};

    if (uthread_scheduler_ && ep) {
        phxrpc::UThreadTcpStream socket;
        bool open_ret{phxrpc::PhxrpcTcpUtils::Open(uthread_scheduler_, &socket, ep->ip, ep->port,
                global_kvraftclientuthread_config_.GetConnectTimeoutMS(),
                *(global_kvraftclientuthread_monitor_.get()))};
        if (open_ret) {
            socket.SetTimeout(global_kvraftclientuthread_config_.GetSocketTimeoutMS());
            phxrpc::HttpMessageHandlerFactory http_msg_factory;
            KVRaftStub stub(socket, *(global_kvraftclientuthread_monitor_.get()), http_msg_factory);
            return stub.AppendEntries(req, resp);
        }
    }

    return -1;
}

int KVRaftClientUThread::Command(const kvraft::KVArgs &req, kvraft::KVReply *resp)
{
    const phxrpc::Endpoint_t *ep{global_kvraftclientuthread_config_.GetRandom()};

    if (uthread_scheduler_ && ep) {
        phxrpc::UThreadTcpStream socket;
        bool open_ret{phxrpc::PhxrpcTcpUtils::Open(uthread_scheduler_, &socket, ep->ip, ep->port,
                global_kvraftclientuthread_config_.GetConnectTimeoutMS(),
                *(global_kvraftclientuthread_monitor_.get()))};
        if (open_ret) {
            socket.SetTimeout(global_kvraftclientuthread_config_.GetSocketTimeoutMS());
            phxrpc::HttpMessageHandlerFactory http_msg_factory;
            KVRaftStub stub(socket, *(global_kvraftclientuthread_monitor_.get()), http_msg_factory);
            return stub.Command(req, resp);
        }
    }

    return -1;
}

