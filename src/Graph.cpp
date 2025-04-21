#include "Graph.h"
#include <iostream>
#include "openalex.h"

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
    size_t a = add_node(from_id, "<title>");
    size_t b = add_node(to_id,   "<title>");
    dir_.push_back({a, b});
    nodes_[a].nbr_.push_back(ref(nodes_[b])); // undirected link a->b
    nodes_[b].nbr_.push_back(ref(nodes_[a])); // undirected link b->a
}

// bfs: returns node indices in visit order from 'start'
vector<size_t> Graph::bfs(size_t start) const {
    vector<size_t> order;
    vector<char>   seen(nodes_.size());
    queue<size_t>  q;
    seen[start] = 1; q.push(start);

    while (!q.empty()) {
        size_t u = q.front(); q.pop();
        order.push_back(u);
        for (auto &nbr : nodes_[u].adjacent()) {
            size_t v = &nbr.get() - &nodes_[0];
            if (!seen[v]) {
                seen[v] = 1;
                q.push(v);
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

    distance[src] = 0; q.push(src);
    while (!q.empty() && distance[target] == -1) {
        size_t u = q.front(); q.pop();
        for (auto &nbr : nodes_[u].adjacent()) {
            size_t v = &nbr.get() - &nodes_[0];
            if (distance[v] == -1) {
                distance[v] = distance[u] + 1;
                prev[v]     = u;
                q.push(v);
            }
        }
    }
    if (distance[target] == -1) return {};

    vector<size_t> path;
    for (size_t v = target; v != src; v = prev[v])
        path.push_back(v);
    path.push_back(src);
    reverse(path.begin(), path.end());
    return path;
}

// Graph ctor: BFS over references only (no citations) to build minimal graph
Graph::Graph(httplib::SSLClient& cli,
             const string& start_id,
             const string& target_id)
{
    // references only go from newer → older
    json start = get_work(cli, start_id);
    json target = get_work(cli, target_id);

    int year_start = start.value("publication_year", 0);
    int year_target = target.value("publication_year", 0);

    if (year_start < year_target) {
        cout << "Error: Start paper must be newer than end paper.\n";
        return;
    }
    unordered_set<string> not_fetched;
    unordered_map<string, unordered_set<string>> fetched_refs;
    unordered_map<string, size_t> distance;
    unordered_map<string, string> prev;
    queue<string> q;

    // initialize
    distance[start_id] = 0;
    q.push(start_id);
    not_fetched.insert(start_id);
    get_refs(cli, not_fetched, fetched_refs);

    size_t iter = 0;
    while (!q.empty()
           && !distance.count(target_id)
           && iter++ < max_size)
    {
        string u = q.front(); q.pop();

        if (not_fetched.erase(u))
            get_refs(cli, not_fetched, fetched_refs);

        // traverse references only
        for (auto &v : fetched_refs[u]) {
            if (!distance.count(v)) {
                distance[v] = distance[u] + 1;
                prev[v]     = u;
                q.push(v);
                not_fetched.insert(v);
            }
        }
    }

    // if no path found, leave graph empty
    if (!distance.count(target_id))
        return;

    // reconstruct ID path
    vector<string> id_path;
    for (string v = target_id; v != start_id; v = prev[v])
        id_path.push_back(v);
    id_path.push_back(start_id);
    reverse(id_path.begin(), id_path.end());

    // fetch titles once for entire path
    vector<string> titles = get_titles(cli, id_path);

    // build local Graph nodes and edges along that path
    add_node(id_path[0], titles[0]);
    for (size_t i = 1; i < id_path.size(); ++i) {
        add_node(id_path[i], titles[i]);
        add_edge(id_path[i-1], id_path[i]);
    }
}
