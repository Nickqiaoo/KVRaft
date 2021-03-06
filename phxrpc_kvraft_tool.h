/* phxrpc_kvraft_tool.h

 Generated by phxrpc_pb2tool from kvraft.proto

 Please DO NOT edit unless you know exactly what you are doing.

*/

#pragma once


namespace phxrpc {


class OptMap;


}


class KVRaftTool {
  public:
    KVRaftTool();
    virtual ~KVRaftTool();

    virtual int PHXEcho(phxrpc::OptMap &bigmap);
    virtual int RequestVote(phxrpc::OptMap &bigmap);
    virtual int AppendEntries(phxrpc::OptMap &bigmap);
    virtual int Command(phxrpc::OptMap &bigmap);

    typedef int (KVRaftTool::*ToolFunc_t)(phxrpc::OptMap &);

    typedef struct tagName2Func {
        const char *name;
        KVRaftTool::ToolFunc_t func;
        const char *opt_string;
        const char *usage;
    } Name2Func_t;

    static Name2Func_t *GetName2Func() {
        static Name2Func_t name2func[]{
            {"PHXEcho", &KVRaftTool::PHXEcho, "c:f:vs:",
                    "-s <string>"},
            {nullptr, nullptr}
        };

        return name2func;
    };
};

