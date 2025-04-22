#include "Graph.h"
#include <iostream>
#include "openalex.h"
#include <chrono>

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

// Graph constructed through BFS over references only (no citations) to build minimal graph
// these IDs may be DOIs
void Graph::graph_by_bfs(httplib::SSLClient& cli,
             const string& start_id_in,
             const string& target_id_in)
{
    auto start_time = chrono::high_resolution_clock::now();
    // references only go from newer → older
    json start = get_work(cli, start_id_in);
    json target = get_work(cli, target_id_in);

    int year_start = start.value("publication_year", 0);
    int year_target = target.value("publication_year", 0);

    string start_id = start["id"];
    string target_id = target["id"];

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
    get_refs(cli, not_fetched, fetched_refs, year_target);

    size_t iter = 0;
    while (!q.empty()
           && !distance.count(target_id)
           && iter++ < max_size)
    {
        string u = q.front(); q.pop();

        if (not_fetched.contains(u))
            get_refs(cli, not_fetched, fetched_refs, year_target);

        // traverse references only
        for (auto &v : fetched_refs[u]) {
            if (!distance.contains(v)) {
                distance[v] = distance[u] + 1;
                prev[v]     = u;
                q.push(v);
                not_fetched.insert(v);
            }
        }
    }

    // if no path found, leave graph empty
    if (!distance.contains(target_id))
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
    this->add_node(id_path[0], titles[0]);
    for (size_t i = 1; i < id_path.size(); ++i) {
        this->add_node(id_path[i], titles[i]);
        this->add_edge(id_path[i-1], id_path[i]);
    }
    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
    cout << "Time elapsed: " << duration.count() << " ms" << endl;
}

// Graph constructed through BeFS over references only (no citations) to build minimal graph
// these IDs may be DOIs
// Greedy heuristic relies on similariy in concepts field with target
void Graph::graph_by_befs(httplib::SSLClient& cli,
                          const string& start_id_in,
                          const string& target_id_in)
{
    auto start_time = chrono::high_resolution_clock::now();

    json start = get_work(cli, start_id_in);
    json target = get_work(cli, target_id_in);

    int year_start = start.value("publication_year", 0);
    int year_target = target.value("publication_year", 0);

    if (year_start < year_target) {
        cout << "Error: Start paper must be newer than end paper.\n";
        return;
    }

    json start_concepts = start["concepts"];
    size_t start_num_refs = start["referenced_works"].size();

    unordered_map<string, float> target_concepts;
    for (const auto& concept_j : target["concepts"])
        target_concepts.emplace(concept_j["id"], concept_j.value("score", 0.0f));

    string start_id = start["id"];
    string target_id = target["id"];
    float start_h = get_heuristic(start_concepts, start_num_refs, target_concepts);

    auto cmp_heuristic = [](const pair<float, string>& left, const pair<float, string>& right) {
        return left.first < right.first;
    };

    unordered_map<string, size_t> distance;
    unordered_map<string, string> prev;
    unordered_map<string, size_t> depth_from_start;
    unordered_set<string> visited;

    priority_queue<
        pair<float, string>,
        vector<pair<float, string>>,
        decltype(cmp_heuristic)
    > q(cmp_heuristic);

    // Initialize
    q.push({start_h, start_id});
    distance[start_id] = 0;
    depth_from_start[start_id] = 0;

    size_t restart_limit = 5;
    size_t restarts = 0;

    while (!q.empty() && !distance.count(target_id)) {
        auto [heuristic, u_id] = q.top(); q.pop();
        if (visited.contains(u_id)) continue;
        visited.insert(u_id);

        size_t depth = depth_from_start[u_id];
        if (depth > max_depth) {
            if (restarts++ < restart_limit) {
                cout << "Restarting from source\n";
                q = decltype(q)(cmp_heuristic); // clear queue
                visited.clear();
                q.push({start_h, start_id});
                continue;
            }
                cout << "Exceeded restart limit\n";
                return;
        }

        auto vs = get_refs_befs(cli, u_id, target_concepts, year_target);
        for (const auto& [v_h, v_id] : vs) {
            if (!visited.contains(v_id) && !distance.contains(v_id)) {
                distance[v_id] = distance[u_id] + 1;
                prev[v_id] = u_id;
                depth_from_start[v_id] = depth + 1;
                q.push({v_h, v_id});
            }
        }
    }

    if (!distance.count(target_id)) return;

    // Reconstruct ID path
    vector<string> id_path;
    for (string v = target_id; v != start_id; v = prev[v])
        id_path.push_back(v);
    id_path.push_back(start_id);
    reverse(id_path.begin(), id_path.end());

    // Fetch titles once
    vector<string> titles = get_titles(cli, id_path);
    this->add_node(id_path[0], titles[0]);

    for (size_t i = 1; i < id_path.size(); ++i) {
        this->add_node(id_path[i], titles[i]);
        this->add_edge(id_path[i - 1], id_path[i]);
    }

    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
    cout << "Time elapsed: " << duration.count() << " ms" << endl;
}

