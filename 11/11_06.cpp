#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/properties.hpp>

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

namespace {

constexpr std::size_t kVertexCount = 10;

using Graph = boost::adjacency_matrix<
    boost::undirectedS,
    boost::no_property,
    boost::property<boost::edge_weight_t, int>
>;

Graph make_complete_random_graph() {
    Graph graph(kVertexCount);

    std::random_device rd;
    std::default_random_engine engine(rd());
    std::uniform_int_distribution<int> distribution(1, 10);

    for (std::size_t u = 0; u < kVertexCount; ++u) {
        for (std::size_t v = u + 1; v < kVertexCount; ++v) {
            boost::add_edge(u, v, distribution(engine), graph);
        }
    }

    return graph;
}

int edge_cost(const Graph& graph, std::size_t u, std::size_t v) {
    const auto [edge_descriptor, exists] = boost::edge(u, v, graph);

    if (!exists) {
        throw std::runtime_error("Edge does not exist between vertices " + std::to_string(u) + " and " + std::to_string(v));
    }

    return boost::get(boost::edge_weight, graph, edge_descriptor);
}

void print_adjacency_matrix(const Graph& graph) {
    std::cout << std::setw(4) << ' ';
    for (std::size_t v = 0; v < kVertexCount; ++v) {
        std::cout << std::setw(4) << v;
    }
    std::cout << '\n';

    for (std::size_t u = 0; u < kVertexCount; ++u) {
        std::cout << std::setw(4) << u;

        for (std::size_t v = 0; v < kVertexCount; ++v) {
            if (u == v) {
                std::cout << std::setw(4) << 0;
            } else {
                std::cout << std::setw(4) << edge_cost(graph, u, v);
            }
        }

        std::cout << '\n';
    }
}

std::pair<std::vector<std::size_t>, int>
solve_tsp_bruteforce(const Graph& graph, std::size_t start_vertex = 0) {
    std::vector<std::size_t> permutation(kVertexCount - 1);
    std::iota(permutation.begin(), permutation.end(), 1);

    std::vector<std::size_t> best_cycle;
    int best_cost = std::numeric_limits<int>::max();

    do {
        int current_cost = 0;
        std::size_t previous = start_vertex;

        for (std::size_t vertex : permutation) {
            current_cost += edge_cost(graph, previous, vertex);
            previous = vertex;
        }

        current_cost += edge_cost(graph, previous, start_vertex);

        if (current_cost < best_cost) {
            best_cost = current_cost;

            best_cycle.clear();
            best_cycle.push_back(start_vertex);
            best_cycle.insert(best_cycle.end(), permutation.begin(), permutation.end());
            best_cycle.push_back(start_vertex);
        }

    } while (std::next_permutation(permutation.begin(), permutation.end()));

    return {best_cycle, best_cost};
}

void print_best_cycle(const std::vector<std::size_t>& cycle, int total_cost) {

    for (std::size_t i = 0; i < cycle.size(); ++i) {
        std::cout << cycle[i];
        if (i + 1 < cycle.size()) {
            std::cout << " -> ";
        }
    }

    std::cout << "total cost is: " << total_cost << '\n';
}

} // namespace

int main() {
    try {
        const Graph graph = make_complete_random_graph();

        print_adjacency_matrix(graph);

        const auto [best_cycle, best_cost] = solve_tsp_bruteforce(graph, 0);

        print_best_cycle(best_cycle, best_cost);
    }
    catch (const std::exception& ex) {
        std::cerr << "error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}