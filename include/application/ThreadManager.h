#ifndef RIDEBOT_THREADMANAGER_H
#define RIDEBOT_THREADMANAGER_H

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace OSBot {

class ThreadManager {
public:
    ThreadManager();
    ~ThreadManager();

    void create_thread();
    void join_all();

private:
    std::vector<std::thread> threads;
    std::mutex mutex;
    std::condition_variable cv;
};

} // namespace OSBot

#endif //RIDEBOT_THREADMANAGER_H
