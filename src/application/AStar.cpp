#include "application/AStar.h"
#include "domain/Environment.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace OSBot {
namespace AStar {

struct Node {
  int y, x;
  int parent_y, parent_x;
  float g_cost, h_cost, f_cost;
};

bool is_valid(int y, int x, int height, int width) {
  return y >= 0 && y < height && x >= 0 && x < width;
}

float calculate_h_value(int y, int x, const Point &end) {
  return std::sqrt(std::pow(y - end.y, 2) + std::pow(x - end.x, 2));
}

Route find_path(const Point &start, const Point &end,
                const Environment &environment) {
  int height = environment.getHeight();
  int width = environment.getWidth();

  // No need to build grid - query Environment directly in is_unblocked

  std::vector<Node> open_list;
  std::vector<std::vector<Node>> closed_list(height, std::vector<Node>(width));
  std::vector<std::vector<bool>> closed_list_bool(
      height, std::vector<bool>(width, false));

  Node start_node;
  start_node.y = start.y;
  start_node.x = start.x;
  start_node.parent_y = -1;
  start_node.parent_x = -1;
  start_node.g_cost = 0;
  start_node.h_cost = calculate_h_value(start.y, start.x, end);
  start_node.f_cost = start_node.g_cost + start_node.h_cost;

  open_list.push_back(start_node);

  while (!open_list.empty()) {
    Node current_node = open_list[0];
    int current_index = 0;
    for (int i = 1; i < open_list.size(); i++) {
      if (open_list[i].f_cost < current_node.f_cost) {
        current_node = open_list[i];
        current_index = i;
      }
    }

    open_list.erase(open_list.begin() + current_index);
    closed_list[current_node.y][current_node.x] = current_node;
    closed_list_bool[current_node.y][current_node.x] = true;

    if (current_node.y == end.y && current_node.x == end.x) {
      Route path;
      Node current = current_node;
      while (current.parent_y != -1) {
        path.push_back({(double)current.x, (double)current.y});
        current = closed_list[current.parent_y][current.parent_x];
      }
      // NO incluir la posición de inicio en la ruta
      // path.push_back({(double)start.x, (double)start.y}); ← REMOVIDO
      std::reverse(path.begin(), path.end());
      return path;
    }

    for (int i = -1; i <= 1; i++) {
      for (int j = -1; j <= 1; j++) {
        if (i == 0 && j == 0)
          continue;
        // Solo movimientos 4-conectados (sin diagonales)
        if (i != 0 && j != 0)
          continue;

        int new_y = current_node.y + i;
        int new_x = current_node.x + j;

        Point test_pos(new_x, new_y);

        // Consultar directamente al Environment si la posición está libre
        if (is_valid(new_y, new_x, height, width) &&
            environment.isPositionFree(test_pos) &&
            !closed_list_bool[new_y][new_x]) {
          float g_new = current_node.g_cost + 1.0f;
          float h_new = calculate_h_value(new_y, new_x, end);
          float f_new = g_new + h_new;

          bool found_in_open = false;
          for (auto &node : open_list) {
            if (node.y == new_y && node.x == new_x) {
              found_in_open = true;
              if (g_new < node.g_cost) {
                node.g_cost = g_new;
                node.f_cost = f_new;
                node.parent_y = current_node.y;
                node.parent_x = current_node.x;
              }
              break;
            }
          }

          if (!found_in_open) {
            Node new_node;
            new_node.y = new_y;
            new_node.x = new_x;
            new_node.parent_y = current_node.y;
            new_node.parent_x = current_node.x;
            new_node.g_cost = g_new;
            new_node.h_cost = h_new;
            new_node.f_cost = f_new;
            open_list.push_back(new_node);
          }
        }
      }
    }
  }

  return {}; // No path found
}
} // namespace AStar
} // namespace OSBot
