#ifndef RIDEBOT_ROUTE_H
#define RIDEBOT_ROUTE_H

#include <vector>

struct Point {
    double x;
    double y;
};

using Route = std::vector<Point>;

#endif //RIDEBOT_ROUTE_H
