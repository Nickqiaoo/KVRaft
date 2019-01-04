#pragma once

#include "kvraft.pb.h"
#include "phxrpc/network.h"

#include <mutex>
#include <string>
#include <vector>
#include <thread>

using std::string;
using std::vector;

namespace raftkv {

using namespace phxrpc;

enum NodeState { Follower, Candidate, Leader };

struct LogEntry {
    enum op { GET, PUT, DEL };
    string key;
    string value;
    int term;
};

class Raft {
   public:
    Raft(int me);
    ~Raft();
    std::pair<int, bool> GetState();
    int AppendEntries(const kvraft::AppendEntriesArgs &req,
                      kvraft::AppendEntriesReply *resp);

    int RequestVote(const kvraft::RequestVoteArgs &req,
                    kvraft::RequestVoteReply *resp);
    void SendRequestVotesToAll(const kvraft::RequestVoteArgs &req);
    int SendAppendEntries(int server, const kvraft::RequestVoteArgs &req,
                          kvraft::RequestVoteReply *resp);
    void SendAppendEntriesToAll();
    int HandleAppendEntries(int server, const kvraft::RequestVoteReply &resp);
    void HandleRequestVote(const kvraft::RequestVoteReply &resp);
    void CommitLog();
    bool Start(kvraft::Operation);
    void HandleTimeout(UThreadSocket_t *socket);
    void ResetTimer();
    void RunTimer();

   private:
    std::mutex raft_mutex_;
    int me_;
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
};

}  // namespace raftkv