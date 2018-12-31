#include "raft.h"

#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <cstdlib>
#include <ctime>

namespace kvraft {

Raft::Raft(phxrpc::UThreadEpollScheduler *const scheduler)
    : scheduler_(scheduler),
      timer_fd_(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK)),
      thread_(&Raft::CreateTimer, this) {}

Raft::~Raft() {}

void Raft::CreateTimer() {
    struct itimerspec timeout;
    std::srand(std::time(nullptr));
    timeout.it_interval.tv_sec = 0;
    timeout.it_interval.tv_nsec = 0;
    timeout.it_value.tv_sec = 0;
    timeout.it_value.tv_nsec = (std::rand() % 150 + 150) * 1000000;
    timerfd_settime(timer_fd_, 0, &timeout, nullptr);

    int epollfd;
    epollfd = epoll_create(1);
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = timer_fd_;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, timer_fd_, &ev);
    struct epoll_event *events;
    for (;;) {
        epoll_wait(epollfd, events, 1, -1);
        
    }
    close(epollfd);
}

void Raft::ResetTimer() {}

}  // namespace kvraft
