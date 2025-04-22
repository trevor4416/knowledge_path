#ifndef OPENALEX_H
#define OPENALEX_H

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "cpp-httplib/httplib.h"
#include "json/single_include/nlohmann/json.hpp"

using json = nlohmann::json;
using namespace std;

// OpenAlex API helper functions. All take an httplib::SSLClient&.

// Search works by text; returns up to num_results results.
json search_works(httplib::SSLClient& cli,
                  const string& search_text,
                  int num_results);

// Get a single work by OpenAlex ID or DOI.
// Note: one API call per invocation; not intended for bulk retrieval.
json get_work(httplib::SSLClient& cli,
              const string& id);

// Fetch outgoing reference IDs for works in not_fetched.
// Stores results in fetched_refs; skips works older than year_target.
void get_refs(httplib::SSLClient& cli,
              unordered_set<string>& not_fetched,
              unordered_map<string, unordered_set<string>>& fetched_refs,
              int year_target);

// Retrieve titles for a list of work IDs.
vector<string> get_titles(httplib::SSLClient& cli,
                          const vector<string>& ids);

// Compute heuristic score for Best-First Search.
float get_heuristic(const json& current_concepts,
                    size_t current_num_refs,
                    const unordered_map<string, float>& target_concepts);

// Get references with heuristic scores for Best-First Search.
vector<pair<float, string>> get_refs_befs(httplib::SSLClient& cli,
                                          const string& id,
                                          const unordered_map<string, float>& target_concepts,
                                          int year_target);

#endif // OPENALEX_H