#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <queue>
#include <algorithm>

using namespace std;

// represents one OpenAlex paper in the graph
struct Node {
    Node(string id, string title) : id_(move(id)), title_(move(title)) {}
    // id string (e.g. OpenAlex "W..." URI)
    const string &get_id()    const { return id_; }
    // human‑readable title
    const string &get_title() const { return title_; }
    // neighbours for traversal (undirected view)
    const vector<reference_wrapper<Node>> &adjacent() const { return nbr_; }
private:
    string id_, title_;
    vector<reference_wrapper<Node>> nbr_;
    friend class Graph;
};

// adjacency‑list graph, stores directed edges for rendering
class Graph {
public:
    // add paper if missing, return index
    size_t add_node(const string &id, const string &title);
    // add citation edge  from -> to  (stores both directions for traversal)
    void   add_edge(const string &from_id, const string &to_id);
    // breadth‑first enumeration starting from index
    vector<size_t> bfs(size_t start) const;
    // shortest unweighted path; empty vector == no path
    vector<size_t> shortest_path(size_t src, size_t dst) const;
    // read‑only helpers
    const vector<Node>               &nodes()          const { return nodes_; }
    const vector<pair<size_t,size_t>> &directed_edges() const { return dir_;   }
private:
    vector<Node>                       nodes_;
    unordered_map<string,size_t>       idx_;
    vector<pair<size_t,size_t>>        dir_; // true direction for rendering
};

#endif
