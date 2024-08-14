/*
 * Copyright 2022 DeepMind Technologies Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SEARCH_BEST_FIRST_SEARCH_H_
#define SEARCH_BEST_FIRST_SEARCH_H_

#include <chrono>
#include <memory>
#include <optional>

#include "heuristics/heuristic.h"
#include "pushworld_puzzle.h"
#include "search/priority_queue.h"
#include "search/random_action_iterator.h"
#include "search/search.h"

#define MEASURE_TIME(t_s, t_e, t)                                              \
  t_e = std::chrono::high_resolution_clock::now();                             \
  t = std::chrono::duration_cast<std::chrono::duration<double>>(t_e - t_s);

namespace pushworld {
namespace search {

/**
 * Searches for a solution to the given `puzzle` by prioritizing the exploration
 * of states that the `heuristic` estimates to have the minimum estimated cost
 * to reach the goal. Returns `std::nullopt` if no solution exists.
 *
 * The `frontier` priority queue is used to track which unexplored states have
 * the minimum estimated cost. In some cases, the type of this priority queue
 * may be chosen to optimize for the `Cost` type (e.g. if costs are discrete or
 * continuous). The `frontier` is cleared when the search begins.
 *
 * `visited` stores all states that are encountered during the search. It is
 * cleared when the search begins.
 */
template <typename Cost>
std::optional<Plan> best_first_search(
    const PushWorldPuzzle &puzzle, heuristic::Heuristic<Cost> &heuristic,
    priority_queue::PriorityQueue<std::shared_ptr<SearchNode>, Cost> &frontier,
    StateSet &visited) {
  const auto &initial_state = puzzle.getInitialState();
  int expansions = 0;

  std::chrono::high_resolution_clock::time_point t_s =
      std::chrono::high_resolution_clock::now();
  std::chrono::high_resolution_clock::time_point t_e;
  std::chrono::duration<double> t;

  if (puzzle.satisfiesGoal(initial_state)) {
    return Plan(); // The plan to reach the goal has no actions.
  }

  RandomActionIterator action_iterator;

  visited.clear();
  visited.insert(initial_state);

  std::vector<int> all_object_indices(initial_state.size());
  for (int i = 0; i < initial_state.size(); i++) {
    all_object_indices[i] = i;
  }
  const RelativeState initial_relative_state{initial_state,
                                             std::move(all_object_indices)};

  frontier.clear();
  frontier.push(std::make_shared<SearchNode>(nullptr, initial_state), 
                heuristic.estimate_cost_to_goal(initial_relative_state));

  MEASURE_TIME(t_s, t_e, t);

  while (!frontier.empty()) {
    const auto parent_node = frontier.top();
    frontier.pop();

    expansions++;

    for (const auto &action : action_iterator.next()) {
      const RelativeState relative_state =
          puzzle.getNextState(parent_node->state, action);

      // Ignore the state if it was already visited.
      if (visited.find(relative_state.state) == visited.end()) {
        const auto node =
            std::make_shared<SearchNode>(parent_node, relative_state.state);

        if (puzzle.satisfiesGoal(relative_state.state)) {
          MEASURE_TIME(t_s, t_e, t);
          // Return the first solution found.
          auto plan = backtrackPlan(puzzle, node);
          std::cout << "Goal found!" << std::endl;
          std::cout << "expansions=" << expansions << std::endl;
          std::cout << "plan_length=" << plan.size() << std::endl;
          std::cout << "time=" << t.count() << std::endl;
          return plan;
        }

        frontier.push(node, heuristic.estimate_cost_to_goal(relative_state));
        visited.insert(relative_state.state);
      }
    }
  }

  // No solution found
  return std::nullopt;
}

/* Identical to `best_first_search` above, but without the `visited` argument.
 */
template <typename Cost>
std::optional<Plan> best_first_search(
    const PushWorldPuzzle &puzzle, heuristic::Heuristic<Cost> &heuristic,
    priority_queue::PriorityQueue<std::shared_ptr<SearchNode>, Cost>
        &frontier) {
  StateSet visited;
  return best_first_search<Cost>(puzzle, heuristic, frontier, visited);
}

} // namespace search
} // namespace pushworld

#endif /* SEARCH_BEST_FIRST_SEARCH_H_ */
