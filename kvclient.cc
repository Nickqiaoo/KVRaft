#include "kvraft.pb.h"
#include "kvraft_client.h"
#include "phxrpc/network.h"

#include <iostream>

using namespace phxrpc;

void printres(kvraft::KVResult res) {
    switch (res) {
        case 0:
            std::cout << "OK" << std::endl;
            break;
        case 1:
            std::cout << "ErrNoKey" << std::endl;
            break;
        case 2:
            std::cout << "ErrTimeout" << std::endl;
            break;
        case 3:
            std::cout << "ErrWrongLeader" << std::endl;
            break;
    }
}

int main() {
    KVRaftClient client;
    KVRaftClient::Init("kvraft_client.conf");

    kvraft::KVArgs arg;
    kvraft::KVReply resp;
    kvraft::Operation* op = new kvraft::Operation;
    int seq = 1;

    op->set_op(kvraft::Operation::PUT);
    op->set_key("key");
    op->set_value("value");
    arg.set_allocated_command(op);
    arg.set_clientid(1);
    arg.set_seq(seq++);
    client.Command(arg, &resp);
    std::cout<<"PUT ";
    printres(resp.res());

    op->set_op(kvraft::Operation::GET);
    arg.set_seq(seq++);
    client.Command(arg, &resp);
    std::cout<<"GET ";
    printres(resp.res());
    std::cout << resp.value() << std::endl;

    op->set_op(kvraft::Operation::DEL);
    arg.set_seq(seq++);
    client.Command(arg, &resp);
    std::cout<<"DEL ";
    printres(resp.res());

    return 0;
}