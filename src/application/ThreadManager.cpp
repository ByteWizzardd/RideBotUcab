#include "application/ThreadManager.h"

namespace OSBot {

ThreadManager::ThreadManager() = default;

ThreadManager::~ThreadManager() {
    join_all();
}

void ThreadManager::create_thread() {
    // Placeholder for thread creation
}

void ThreadManager::join_all() {
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

} // namespace OSBot
