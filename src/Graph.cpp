#include "Graph.h"

// look up or insert a new node, return its index
size_t Graph::add_node(const string& id, const string& title) {
    auto it = idx_.find(id);
    if (it == idx_.end()) {
        nodes_.emplace_back(id, title);
        size_t i = nodes_.size() - 1;
        idx_[id] = i;
        return i;
    }
    return it->second;
}

// record directed edge and link nodes for traversal
void Graph::add_edge(const string& from_id, const string& to_id) {
    size_t a = add_node(from_id, "<title>"); // ensure source exists
    size_t b = add_node(to_id,   "<title>"); // ensure target exists
    dir_.push_back({a, b});
    nodes_[a].nbr_.push_back(ref(nodes_[b])); // undirected link a->b
    nodes_[b].nbr_.push_back(ref(nodes_[a])); // undirected link b->a
}

// bfs: returns node indices in visit order from 'start'
vector<size_t> Graph::bfs(size_t start) const {
    vector<size_t> order;
    vector<char>   seen(nodes_.size());
    queue<size_t>  q;
    seen[start] = 1; q.push(start);          // mark and enqueue

    while (!q.empty()) {
        size_t u = q.front(); q.pop();
        order.push_back(u);                  // record visit
        for (auto &nbr : nodes_[u].adjacent()) {
            size_t v = &nbr.get() - &nodes_[0];
            if (!seen[v]) {
                seen[v] = 1;
                q.push(v);                  // enqueue neighbor
            }
        }
    }
    return order;
}

// shortest_path: unweighted BFS from src to dst, empty if none
vector<size_t> Graph::shortest_path(size_t src, size_t target) const {
    if (src >= nodes_.size() || target >= nodes_.size()) return {};

    vector<int>    distance(nodes_.size(), -1);
    vector<size_t> prev(nodes_.size());
    queue<size_t>  q;

    distance[src] = 0; q.push(src);               // start BFS
    while (!q.empty() && distance[target] == -1) {
        size_t u = q.front(); q.pop();
        for (auto &nbr : nodes_[u].adjacent()) {
            size_t v = &nbr.get() - &nodes_[0];
            if (distance[v] == -1) {
                distance[v] = distance[u] + 1;  // set distance
                prev[v] = u;
                q.push(v);
            }
        }
    }
    if (distance[target] == -1) return {};

    // reconstruct path
    vector<size_t> path;
    for (size_t v = target; v != src; v = prev[v])
        path.push_back(v);
    path.push_back(src);
    reverse(path.begin(), path.end());  // src->dst order
    return path;
}