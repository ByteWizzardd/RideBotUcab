#include "infrastructure/Storage.h"
#include "domain/Environment.h"
#include "domain/Robot.h"
#include "application/TaskScheduler.h"
#include <cassert>
#include <iostream>
#include <filesystem>
#include <vector>

void test_storage() {
    std::cout << "Running Storage Test...\n";

    // 1. Setup Environment
    OSBot::Environment env(20, 15);
    env.initialize(); // Random obstacles
    env.clearAllObstacles();
    
    // Add specific obstacles
    env.toggleObstacle(OSBot::Point(5, 5));
    env.toggleObstacle(OSBot::Point(10, 10));

    // 2. Setup Robots
    std::vector<OSBot::Robot*> robots;
    OSBot::Robot* r1 = new OSBot::Robot(env);
    r1->setId(1);
    r1->setPosition(OSBot::Point(1, 1));
    r1->setBatteryLevel(85.5f);
    robots.push_back(r1);

    OSBot::Robot* r2 = new OSBot::Robot(env);
    r2->setId(2);
    r2->setPosition(OSBot::Point(2, 2));
    r2->setBatteryLevel(42.0f);
    robots.push_back(r2);
    
    // 3. Setup Tasks
    OSBot::TaskScheduler scheduler;
    std::vector<OSBot::Point> waypoints = {OSBot::Point(15, 15)};
    OSBot::Task t1(100, waypoints, OSBot::TaskPriority::HIGH);
    scheduler.add_task(t1);

    // 4. Save
    std::string filename = "test_save.osbt";
    // Cast robots to const vector required by save_state
    std::vector<const OSBot::Robot*> const_robots;
    for(auto* r : robots) const_robots.push_back(r);

    if (OSBot::Storage::save_state(filename, env, const_robots, scheduler)) {
        std::cout << "[PASS] Save successful.\n";
    } else {
        std::cerr << "[FAIL] Save failed.\n";
        exit(1);
    }

    // 5. Clear everything
    env.clearAllObstacles();
    // Delete old robots
    for(auto* r : robots) delete r;
    robots.clear();
    scheduler.clear();

    // 6. Load
    if (OSBot::Storage::load_state(filename, env, robots, scheduler)) {
        std::cout << "[PASS] Load successful.\n";
    } else {
        std::cerr << "[FAIL] Load failed.\n";
        exit(1);
    }

    // 7. Verify
    // Verify Obstacles
    if (!env.isPositionFree(OSBot::Point(5, 5)) && !env.isPositionFree(OSBot::Point(10, 10))) {
        std::cout << "[PASS] Obstacles verified.\n";
    } else {
        std::cerr << "[FAIL] Obstacles verification failed.\n";
    }

    // Verify Robots
    if (robots.size() == 2) {
        bool r1Found = false, r2Found = false;
        for(auto* r : robots) {
            if (r->getId() == 1) {
                if(r->getPosition().x == 1 && r->getPosition().y == 1 && r->getBatteryLevel() == 85.5f) r1Found = true;
            } else if (r->getId() == 2) {
                if(r->getPosition().x == 2 && r->getPosition().y == 2 && r->getBatteryLevel() == 42.0f) r2Found = true;
            }
        }
        if (r1Found && r2Found) std::cout << "[PASS] Robots verified.\n";
        else std::cerr << "[FAIL] Robot data mismatch.\n";
    } else {
        std::cerr << "[FAIL] Robot count mismatch: " << robots.size() << "\n";
    }

    // Verify Tasks
    if (scheduler.getAllTasks().size() == 1) {
        OSBot::Task t = scheduler.getAllTasks()[0];
        if (t.getId() == 100 && t.getPriority() == OSBot::TaskPriority::HIGH) {
            std::cout << "[PASS] Tasks verified.\n";
        } else {
            std::cerr << "[FAIL] Task data mismatch.\n";
        }
    } else {
         std::cerr << "[FAIL] Task count mismatch.\n";
    }

    // Cleanup
    for(auto* r : robots) delete r;
    std::remove(filename.c_str());
}

int main() {
    test_storage();
    return 0;
}
