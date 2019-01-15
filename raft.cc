#include "raft.h"

#include <mutex>

#include <stdint.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <sys/timerfd.h>

namespace raftkv {

using namespace phxrpc;

Raft::Raft(int me, int num_of_server)
    : timer_fd_(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK)),
      me_(me),
      num_of_server_(num_of_server),
      scheduler_(64 * 1024, 300),
      next_index_(num_of_server, 1),
      match_index_(num_of_server, 0),
      client_(&scheduler_),
      thread_(std::bind(&Raft::RunTimer, this)) {
    std::lock_guard<std::mutex> lock(raft_mutex_);
    log_.emplace_back(LogEntry{});
}

Raft::~Raft() { thread_.join(); }

void Raft::RunTimer() {
    struct itimerspec timeout;
    std::srand(Timer::GetSteadyClockMS()+getpid());
    timeout.it_interval.tv_sec = 0;
    timeout.it_interval.tv_nsec = 0;
    timeout.it_value.tv_sec = 0;
    timeout.it_value.tv_nsec = (std::rand() % 150 + 150) * 1000000;
    raft_mutex_.lock();
    timerfd_settime(timer_fd_, 0, &timeout, nullptr);
    raft_mutex_.unlock();
    UThreadSocket_t *socket = scheduler_.CreateSocket(timer_fd_);
    scheduler_.AddTask(std::bind(&Raft::HandleTimeout, this, socket), nullptr);
    scheduler_.Run();
}

void Raft::ResetTimer() {
    printf("%s ", __func__);
    struct itimerspec timeout;
    std::srand(Timer::GetSteadyClockMS()+getpid());
    timeout.it_interval.tv_sec = 0;
    timeout.it_interval.tv_nsec = 0;
    timeout.it_value.tv_sec = 0;
    if (state_ != Leader) {
        timeout.it_value.tv_nsec = (std::rand() % 150 + 150) * 1000000;
        printf("%d\n", timeout.it_value.tv_nsec);
    } else {
        timeout.it_value.tv_nsec = 100000000;
    }
    timerfd_settime(timer_fd_, 0, &timeout, nullptr);
}

void Raft::HandleTimeout(UThreadSocket_t *socket) {
    struct itimerspec timeout;
    uint64_t buf;
    while (1) {
        timerfd_gettime(timer_fd_, &timeout);
        // printf("%d %d\n", timeout.it_value.tv_nsec, timeout.it_value.tv_sec);
        int ret = UThreadRead(*socket, (void *)&buf, sizeof(uint64_t), 0);
        struct itimerspec timeout;
        timerfd_gettime(timer_fd_, &timeout);
        // printf("%d %d\n", timeout.it_value.tv_nsec, timeout.it_value.tv_sec);
        // printf("%d %d\n", ret, buf);
        if (ret > 0) {
            std::lock_guard<std::mutex> lock(raft_mutex_);
            if (state_ != Leader) {
                state_ = Candidate;
                printf("%s become candidate\n", __func__);
                current_term_ += 1;
                voted_for_ = me_;
                sum_of_vote_ = 1;
                kvraft::RequestVoteArgs req;
                req.set_term(current_term_);
                req.set_candidateid(me_);
                req.set_lastlogindex(0);
                req.set_lastlogterm(0);
                if (log_.size() > 1) {
                    req.set_lastlogindex(log_.size() - 1);
                    req.set_lastlogterm(log_[log_.size() - 1].term);
                }
                ResetTimer();
                SendRequestVotesToAll(req);
            } else {
                printf("leader handletimeout\n");
                SendAppendEntriesToAll();
                ResetTimer();
            }
        }
    }
}

void Raft::RequestVote(const kvraft::RequestVoteArgs &req, kvraft::RequestVoteReply *resp) {
    printf("receive RequestVote\n");
    std::lock_guard<std::mutex> lock(raft_mutex_);
    printf("reqterm %d,myterm %d\n",req.term(),current_term_);

    resp->set_term(current_term_);
    resp->set_votegranted(false);

    if (req.term() > current_term_) {
        state_ = Follower;
        current_term_ = req.term();
        voted_for_ = -1;
    }

    if (req.term() >= current_term_) {
        if (voted_for_ == -1 || voted_for_ == req.candidateid()) {
            if (log_.size() == 1 || log_[log_.size() - 1].term < req.lastlogterm() ||
                (log_[log_.size() - 1].term == req.lastlogterm() &&
                 log_.size() - 1 <= req.lastlogindex())) {
                state_ = Follower;
                printf("become follower\n");
                current_term_ = req.term();
                voted_for_ = req.candidateid();
                resp->set_term(current_term_);
                resp->set_votegranted(true);
                ResetTimer();
            }
        }
    }
}

void Raft::SendRequestVotesToAll(const kvraft::RequestVoteArgs &req) {
    printf("%s\n", __func__);
    for (int i = 0; i < num_of_server_; i++) {
        if (i != me_) {
            scheduler_.AddTask(
                [this, req, i](void *) {
                    kvraft::RequestVoteReply resp;
                    int ret = client_.RequestVote(req, &resp, i);
                    if (ret != -1) {
                        HandleRequestVote(resp);
                    }
                },
                nullptr);
        }
    }
}

void Raft::HandleRequestVote(const kvraft::RequestVoteReply &resp) {
    printf("%s\n", __func__);
    std::lock_guard<std::mutex> lock(raft_mutex_);
    if (resp.term() > current_term_) {
        current_term_ = resp.term();
        state_ = Follower;
        printf("%d become follower\n", me_);
        voted_for_ = -1;
        ResetTimer();
        return;
    }
    if (state_ == Candidate && resp.votegranted()) {
        sum_of_vote_ += 1;
        if (sum_of_vote_ >= num_of_server_ / 2 + 1) {
            state_ = Leader;
            printf("%d become leader\n", me_);
            for (int i = 0; i < num_of_server_; i++) {
                if (i != me_) {
                    next_index_[i] = log_.size();
                    match_index_[i] = 0;
                }
            }
            SendAppendEntriesToAll();
            ResetTimer();
        }
    }
}

void Raft::AppendEntries(const kvraft::AppendEntriesArgs &req, kvraft::AppendEntriesReply *resp) {
    std::lock_guard<std::mutex> lock(raft_mutex_);
    printf("receive AppendEntires\n");
    if (req.term() < current_term_) {
        resp->set_success(false);
        resp->set_term(current_term_);
    } else {
        state_ = Follower;
        current_term_ = req.term();
        voted_for_ = -1;
        resp->set_term(req.term());

        if (req.prevlogindex() >= 0 && (log_.size() - 1 < req.prevlogindex() ||
                                        log_[req.prevlogindex()].term != req.prevlogterm())) {
            int replicated_index = log_.size() - 1;
            if (replicated_index > req.prevlogindex()) {
                replicated_index = req.prevlogindex();
            }
            while (replicated_index > 0) {
                if (log_[replicated_index].term == req.prevlogterm()) break;
                replicated_index--;
            }
            resp->set_replicatedindex(replicated_index);
            resp->set_success(false);
        } else if (req.entries_size() > 0) {
            log_.erase(log_.end() - (log_.size() - 1 - req.prevlogindex()), log_.end());
            for (int i = 0; i < req.entries_size(); i++) {
                kvraft::LogEntry l = req.entries(i);
                log_.emplace_back(LogEntry{LogEntry::operation(req.entries(i).command().op()),
                                           req.entries(i).command().key(),
                                           req.entries(i).command().value(),
                                           req.entries(i).term()});
            }
            if (log_.size() - 1 >= req.leadercommit()) {
                commit_index_ = req.leadercommit();
                CommitLog();
            }
            resp->set_replicatedindex(log_.size() - 1);
            resp->set_success(true);
        } else {
            if (log_.size() - 1 >= req.leadercommit()) {
                commit_index_ = req.leadercommit();
                CommitLog();
            }
            resp->set_replicatedindex(req.prevlogindex());
            resp->set_success(true);
        }
    }
    ResetTimer();
}

void Raft::SendAppendEntriesToAll() {
    printf("%s\n", __func__);
    for (int i = 0; i < num_of_server_; i++) {
        if (i != me_) {
            kvraft::AppendEntriesArgs req;
            req.set_term(current_term_);
            req.set_leaderid(me_);
            req.set_prevlogindex(next_index_[i] - 1);
            if (req.prevlogindex() > 0) {
                req.set_prevlogterm(log_[req.prevlogindex()].term);
            }
            if (next_index_[i] <= log_.size() - 1) {
                for (int j = next_index_[i]; j < log_.size(); j++) {
                    kvraft::LogEntry *entry = req.add_entries();
                    kvraft::Operation *op = entry->mutable_command();
                    op->set_key(log_[j].key);
                    op->set_value(log_[j].value);
                    op->set_op(kvraft::Operation_OpName(log_[j].op));
                    entry->set_term(log_[j].term);
                }
            }
            req.set_leadercommit(commit_index_);

            scheduler_.AddTask(
                [this, req, i](void *) {
                    kvraft::AppendEntriesReply resp;
                    printf("send AppendEntries to %d\n",i);
                    int ret = client_.AppendEntries(req, &resp, i);
                    if (ret != -1) {
                        HandleAppendEntries(i, resp);
                    }
                },
                nullptr);
        }
    }
}

void Raft::HandleAppendEntries(int server, const kvraft::AppendEntriesReply &resp) {
    printf("%s\n", __func__);
    std::lock_guard<std::mutex> lock(raft_mutex_);
    if (state_ != Leader) {
        return;
    }

    if (resp.term() > current_term_) {
        current_term_ = resp.term();
        state_ = Follower;
        voted_for_ = -1;
        ResetTimer();
        return;
    }
    if (resp.success()) {
        next_index_[server] = resp.replicatedindex() + 1;
        match_index_[server] = resp.replicatedindex();
        int sum_of_reply = 1;
        for (int i = 0; i < num_of_server_; i++) {
            if (i != me_) {
                if (match_index_[i] >= match_index_[server]) {
                    sum_of_reply += 1;
                }
            }
        }
        if (sum_of_reply >= (num_of_server_ / 2 + 1) && commit_index_ < match_index_[server] &&
            log_[match_index_[server]].term == current_term_) {
            commit_index_ = match_index_[server];
            CommitLog();
        }
    } else {
        next_index_[server] = resp.replicatedindex() + 1;
    }
}

bool Raft::Start(const raftkv::LogEntry::operation &op, const string &key, const string &value) {
    std::lock_guard<std::mutex> lock(raft_mutex_);
    if (state_ != Leader) return false;
    log_.emplace_back(LogEntry{op, key, value, current_term_});
    return true;
}

void Raft::CommitLog() {
    if (commit_index_ > log_.size() - 1) {
        commit_index_ = log_.size() - 1;
    }
    last_applied_ = commit_index_;
}

}  // namespace raftkv
