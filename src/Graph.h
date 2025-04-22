#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <queue>
#include <algorithm>
#include "openalex.h"

using namespace std;

// represents one OpenAlex paper in the graph
struct Node {
    Node(string id, string title) : id_(move(id)), title_(move(title)) {}
    const string &get_id()    const { return id_; }
    const string &get_title() const { return title_; }
    const vector<reference_wrapper<Node>> &adjacent() const { return nbr_; }
private:
    string id_, title_;
    vector<reference_wrapper<Node>> nbr_;
    friend class Graph;
};

// adjacency‑list graph, stores directed edges for rendering
class Graph {
public:
    size_t add_node(const string &id, const string &title);
    void   add_edge(const string &from_id, const string &to_id);
    vector<size_t> bfs(size_t start) const;
    vector<size_t> shortest_path(size_t src, size_t dst) const;
    void  graph_by_bfs(httplib::SSLClient& cli, const string &start_id_in, const string &target_id_in);
    void  graph_by_befs(httplib::SSLClient& cli, const string &start_id_in, const string &target_id_in);
    const vector<Node>               &nodes()          const { return nodes_; }
    const vector<pair<size_t,size_t>> &directed_edges() const { return dir_;   }
    size_t get_size() { return nodes_.size(); }
private:
    size_t                             max_depth = 10;
    size_t                             max_size = 500;
    vector<Node>                       nodes_;
    unordered_map<string,size_t>       idx_;
    vector<pair<size_t,size_t>>        dir_;
};

#endif
