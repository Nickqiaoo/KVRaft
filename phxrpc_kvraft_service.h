/* phxrpc_kvraft_service.h

 Generated by phxrpc_pb2service from kvraft.proto

 Please DO NOT edit unless you know exactly what you are doing.

*/

#pragma once

#include "kvraft.pb.h"


class KVRaftService {
  public:
    KVRaftService();
    virtual ~KVRaftService();

    virtual int PHXEcho(const google::protobuf::StringValue &req, google::protobuf::StringValue *resp);
    virtual int RequestVote(const kvraft::RequestVoteArgs &req, kvraft::RequestVoteReply *resp);
    virtual int AppendEntries(const kvraft::AppendEntriesArgs &req, kvraft::AppendEntriesReply *resp);
    virtual int Command(const kvraft::KVArgs &req, kvraft::KVReply *resp);
};
