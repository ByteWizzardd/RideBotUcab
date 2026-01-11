#ifndef RIDEBOT_ASTAR_H
#define RIDEBOT_ASTAR_H

#include "domain/Global.h"
#include "domain/Route.h"

namespace OSBot {

// Forward declaration
class Environment;

namespace AStar {
Route find_path(const Point &start, const Point &end,
                const Environment &environment);
}
} // namespace OSBot

#endif // RIDEBOT_ASTAR_H
