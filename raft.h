#pragma once
#include <string>
#include <vector>

using namespace std;

enum NodeState { Follower, Candidate, Leader };
struct LogEntry {
    enum op { GET, PUT, DEL };
    string key;
    string value;
};
class Raft {
   public:
   private:
    int CurrentTerm;
    int VotedFor;
    vector<LogEntry> Log;

    int CommitIndex;
    int LastApplied;

    vector<int> NextIndex;
    vector<int> MatchIndex;

    NodeState State;
    int SumOfVote;
};