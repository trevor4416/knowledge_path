#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include "openalex.h"

using namespace std;

// Node represents one OpenAlex paper in the graph
struct Node {
    Node(string id, string title)
        : id_(move(id)), title_(move(title)) {}

    const string& get_id()    const { return id_;    }
    const string& get_title() const { return title_; }
    const vector<reference_wrapper<Node>>& adjacent() const { return nbr_; }

private:
    string id_;
    string title_;
    vector<reference_wrapper<Node>> nbr_;

    friend class Graph;
};

// Graph using an adjacency list; stores directed edges for rendering
class Graph {
public:
    // Add or lookup a node by ID, return its index
    size_t add_node(const string& id, const string& title);

    // Add a directed edge (and link both nodes for traversal)
    void add_edge(const string& from_id, const string& to_id);

    // Build citation graph via BFS over references only
    void graph_by_bfs(httplib::SSLClient& cli,
                      const string& start_id,
                      const string& target_id);

    // Build citation graph via best‑first (heuristic) search
    void graph_by_befs(httplib::SSLClient& cli,
                       const string& start_id,
                       const string& target_id);

    const vector<Node>& nodes() const
    { return nodes_; }

    const vector<pair<size_t, size_t>>& directed_edges() const
    { return dir_; }

    size_t get_size() const
    { return nodes_.size(); }

private:
    size_t max_depth = 10;
    size_t max_size  = 500;

    vector<Node> nodes_;
    unordered_map<string, size_t> idx_;
    vector<pair<size_t, size_t>> dir_;
};

#endif // GRAPH_H