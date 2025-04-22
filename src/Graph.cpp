#include "Graph.h"
#include <iostream>
#include "openalex.h"
#include <chrono>
#include <queue>

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

void Graph::add_edge(const string& from_id, const string& to_id) {
    size_t a = add_node(from_id, "<title>");
    size_t b = add_node(to_id,   "<title>");
    dir_.push_back({a, b});
    nodes_[a].nbr_.push_back(ref(nodes_[b]));
    nodes_[b].nbr_.push_back(ref(nodes_[a]));
}

// Graph constructed through BFS over references only (no citations)
void Graph::graph_by_bfs(httplib::SSLClient& cli,
                         const string& start_id_in,
                         const string& target_id_in)
{
    auto start_time = chrono::high_resolution_clock::now();

    json start   = get_work(cli, start_id_in);
    json target  = get_work(cli, target_id_in);
    int year_s   = start.value("publication_year", 0);
    int year_t   = target.value("publication_year", 0);

    string start_id  = start["id"];
    string target_id = target["id"];

    if (year_s < year_t) {
        cout << "Error: Start paper must be newer than end paper.\n";
        return;
    }

    unordered_set<string> not_fetched;
    unordered_map<string, unordered_set<string>> fetched_refs;
    unordered_map<string, size_t> distance;
    unordered_map<string, string> prev;
    queue<string> q;

    distance[start_id] = 0;
    q.push(start_id);
    not_fetched.insert(start_id);
    get_refs(cli, not_fetched, fetched_refs, year_t);

    size_t iter = 0;
    while (!q.empty() && !distance.count(target_id) && iter++ < max_size) {
        string u = q.front();
        q.pop();
        if (not_fetched.erase(u)) {
            get_refs(cli, not_fetched, fetched_refs, year_t);
        }
        for (auto& v : fetched_refs[u]) {
            if (!distance.count(v)) {
                distance[v] = distance[u] + 1;
                prev[v]      = u;
                q.push(v);
                not_fetched.insert(v);
            }
        }
    }

    if (!distance.count(target_id))
        return;

    vector<string> id_path;
    for (string v = target_id; v != start_id; v = prev[v]) {
        id_path.push_back(v);
    }
    id_path.push_back(start_id);
    reverse(id_path.begin(), id_path.end());

    vector<string> titles = get_titles(cli, id_path);
    add_node(id_path[0], titles[0]);
    for (size_t i = 1; i < id_path.size(); ++i) {
        add_node(id_path[i], titles[i]);
        add_edge(id_path[i - 1], id_path[i]);
    }

    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
    cout << "Time elapsed: " << duration.count() << " ms\n";
}

// Graph constructed through BeFS over references only (no citations)
// Greedy heuristic relies on similarity in 'concepts' field with target.
void Graph::graph_by_befs(httplib::SSLClient& cli,
                          const string& start_id_in,
                          const string& target_id_in)
{
    auto start_time = chrono::high_resolution_clock::now();

    json start  = get_work(cli, start_id_in);
    json target = get_work(cli, target_id_in);
    int year_s   = start.value("publication_year", 0);
    int year_t   = target.value("publication_year", 0);

    if (year_s < year_t) {
        cout << "Error: Start paper must be newer than end paper.\n";
        return;
    }

    json start_concepts   = start["concepts"];
    size_t start_num_refs = start["referenced_works"].size();

    unordered_map<string, float> target_concepts;
    for (const auto& c : target["concepts"]) {
        target_concepts[c["id"]] = c.value("score", 0.0f);
    }

    string start_id  = start["id"];
    string target_id = target["id"];
    float h0         = get_heuristic(start_concepts, start_num_refs, target_concepts);

    auto cmp_h = [](auto& l, auto& r) { return l.first < r.first; };
    priority_queue<pair<float, string>,
                   vector<pair<float, string>>,
                   decltype(cmp_h)> pq(cmp_h);

    unordered_map<string, size_t> distance;
    unordered_map<string, string> prev;
    unordered_map<string, size_t> depth;
    unordered_set<string> visited;

    distance[start_id] = 0;
    depth[start_id]    = 0;
    pq.push({h0, start_id});

    size_t restarts = 0;
    while (!pq.empty() && !distance.count(target_id)) {
        auto [h, u] = pq.top();
        pq.pop();
        if (visited.count(u))
            continue;
        visited.insert(u);

        if (depth[u] > max_depth) {
            if (restarts++ < 5) {
                cout << "Restarting...\n";
                while (!pq.empty())
                    pq.pop();
                visited.clear();
                pq.push({h0, start_id});
                continue;
            }
            cout << "Exceeded restart limit\n";
            return;
        }

        auto vs = get_refs_befs(cli, u, target_concepts, year_t);
        for (auto& [nh, v] : vs) {
            if (!visited.count(v) && !distance.count(v)) {
                distance[v] = distance[u] + 1;
                prev[v]     = u;
                depth[v]    = depth[u] + 1;
                pq.push({nh, v});
            }
        }
    }

    if (!distance.count(target_id))
        return;

    vector<string> id_path;
    for (string v = target_id; v != start_id; v = prev[v]) {
        id_path.push_back(v);
    }
    id_path.push_back(start_id);
    reverse(id_path.begin(), id_path.end());

    vector<string> titles = get_titles(cli, id_path);
    add_node(id_path[0], titles[0]);
    for (size_t i = 1; i < id_path.size(); ++i) {
        add_node(id_path[i], titles[i]);
        add_edge(id_path[i - 1], id_path[i]);
    }

    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
    cout << "Time elapsed: " << duration.count() << " ms\n";
}