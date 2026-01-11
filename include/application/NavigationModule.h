#ifndef NAVIGATION_MODULE_H
#define NAVIGATION_MODULE_H

#include "domain/Global.h"
#include "domain/Route.h"

namespace OSBot {

// Forward declaration
class Environment;

class NavigationModule {
public:
  NavigationModule();
  Route find_route(const Point &start, const Point &end,
                   const Environment &environment);
};

} // namespace OSBot

#endif // NAVIGATION_MODULE_H
