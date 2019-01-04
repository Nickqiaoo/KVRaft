#include "raft.h"
#include "kvraft_client_uthread.h"

#include <mutex>

#include <stdint.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <sys/timerfd.h>

namespace raftkv {

using namespace phxrpc;

Raft::Raft(int me)
    : timer_fd_(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK)),
      me_(me),
      scheduler_(64 * 1024, 300),
      thread_(std::bind(&Raft::RunTimer, this)) {}

Raft::~Raft() { thread_.join(); }

void Raft::RunTimer() {
    struct itimerspec timeout;
    std::srand(std::time(nullptr));
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
    struct itimerspec timeout;
    std::srand(std::time(nullptr));
    timeout.it_interval.tv_sec = 0;
    timeout.it_interval.tv_nsec = 0;
    timeout.it_value.tv_sec = 0;
    if (state_ != Leader) {
        timeout.it_value.tv_nsec = (std::rand() % 150 + 150) * 1000000;
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
                SendAppendEntriesToAll();
                ResetTimer();
            }
        }
    }
}
void Raft::SendRequestVotesToAll(const kvraft::RequestVoteArgs &req) {
    KVRaftClientUThread client(&scheduler_);

    for (int i = 0; i < 3; i++) {
        if (i != me_) {
            scheduler_.AddTask(
                [&client, this, req, i](void *) {
                    kvraft::RequestVoteReply resp;
                    int ret = client.RequestVote(req, &resp, i);
                    if (ret != -1) {
                        HandleRequestVote(resp);
                    }
                },
                nullptr);
        }
    }
}
void Raft::HandleRequestVote(const kvraft::RequestVoteReply &resp){
    //if(resp.term())
}

void Raft::SendAppendEntriesToAll() {}

}  // namespace raftkv
