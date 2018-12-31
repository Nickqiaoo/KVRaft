#pragma once

#include "kvraft_client_uthread.h"

#include <mutex>
#include <string>
#include <vector>

using std::string;
using std::vector;

namespace kvraft {

enum NodeState { Follower, Candidate, Leader };

struct LogEntry {
    enum op { GET, PUT, DEL };
    string key;
    string value;
};

class Raft {
   public:
   Raft(phxrpc::UThreadEpollScheduler *const scheduler);
   ~Raft();
    std::pair<int, bool> GetState();
    int AppendEntries(const kvraft::AppendEntriesArgs &req,
                      kvraft::AppendEntriesReply *resp);

    int RequestVote(const kvraft::RequestVoteArgs &req,
                    kvraft::RequestVoteReply *resp);
    int SendAppendEntries(int server, const kvraft::RequestVoteArgs &req,
                          kvraft::RequestVoteReply *resp);
    int SendAppendEntriesToAll();
    int HandleAppendEntries(int server, kvraft::RequestVoteReply *resp);
    int HandleRequestVote(kvraft::RequestVoteReply *resp);
    void CommitLog();
    bool Start(kvraft::Operation);
    void HandleTimeout();
    void ResetTimer();
    void CreateTimer();
   private:
    std::mutex raft_mutex_;
    int current_term_{0};
    int voted_for_{-1};
    vector<LogEntry> log_;

    int commit_index_{0};
    int last_applied_{0};

    vector<int> next_index_;
    vector<int> match_index_;

    NodeState state_{Follower};
    int sum_of_vote_{0};
    phxrpc::UThreadEpollScheduler *scheduler_{nullptr};
    int timer_fd_;
    std::thread thread_;
};

}  // namespace kvraft