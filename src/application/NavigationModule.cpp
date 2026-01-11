#include "application/NavigationModule.h"
#include "application/AStar.h"
#include "domain/Environment.h"

namespace OSBot {

NavigationModule::NavigationModule() = default;

Route NavigationModule::find_route(const Point &start, const Point &end,
                                   const Environment &environment) {
  return AStar::find_path(start, end, environment);
}

} // namespace OSBot
