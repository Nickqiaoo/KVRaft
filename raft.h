#pragma once

#include "kvraft.pb.h"
#include "phxrpc/network.h"
#include "kvraft_client_uthread.h"

#include <mutex>
#include <string>
#include <thread>
#include <vector>

using std::string;
using std::vector;

namespace raftkv {

using namespace phxrpc;

enum NodeState { Follower, Candidate, Leader };

struct LogEntry {
    enum operation { GET, PUT, DEL };
    operation op;
    string key;
    string value;
    int term;
};

class Raft {
   public:
    Raft(int me, int num_of_server);
    ~Raft();
    std::pair<int, bool> GetState();
    void AppendEntries(const kvraft::AppendEntriesArgs &req, kvraft::AppendEntriesReply *resp);

    void RequestVote(const kvraft::RequestVoteArgs &req, kvraft::RequestVoteReply *resp);
    void SendRequestVotesToAll(const kvraft::RequestVoteArgs &req);
    void SendAppendEntriesToAll();
    void HandleAppendEntries(int server, const kvraft::AppendEntriesReply &resp);
    void HandleRequestVote(const kvraft::RequestVoteReply &resp);
    void CommitLog();
    std::pair<int,bool> Start(const raftkv::LogEntry::operation &op ,const string &key ,const string &value);
    void HandleTimeout(UThreadSocket_t *socket);
    void ResetTimer();
    void RunTimer();

   private:
    std::mutex raft_mutex_;
    int me_;
    int num_of_server_;
    int current_term_{0};
    int voted_for_{-1};
    vector<LogEntry> log_;

    int commit_index_{0};
    int last_applied_{0};

    vector<int> next_index_;
    vector<int> match_index_;

    NodeState state_{Follower};
    int sum_of_vote_{0};
    int timer_fd_;
    std::thread thread_;
    UThreadEpollScheduler scheduler_;
    KVRaftClientUThread client_;

};

}  // namespace raftkv