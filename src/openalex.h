#ifndef OPENALEX_H
#define OPENALEX_H

#endif //OPENALEX_H

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "cpp-httplib/httplib.h"
#include "json/single_include/nlohmann/json.hpp"

using json = nlohmann::json;
using namespace std;

// declaration for helper functions to get data from api.openalex.org

// these helper functions take an httplib::SSLClient as a parameter,
// so that a new client need not be created for every function call
// all return either a json work object or a json array of work objects

// searches for search_text in titles, abstracts, and fulltext, and returns the first num_results results
json search_works(httplib::SSLClient& cli, const string& search_text, int num_results);

// returns the work specified by the openalex ID, the DOI, or other supported external IDs (https://docs.openalex.org/api-entities/works/get-a-single-work)
// DO NOT USE IN IMPLEMENTATION OF FUNCTIONS THAT RETRIEVE MULTIPLE WORKS AT ONCE; IT CALLS THE API ONCE FOR EACH WORK
json get_work(httplib::SSLClient& cli, const string& id);

// gets the full work objects given a json containing an array of their ids, like that returned by get_refs
json get_works(httplib::SSLClient& cli, const json ids_json);

// gets the ids of works referenced by those in not_fetched and places them in fetched_refs respectively;
// also clears not_fetched
// outgoing edges in citation graph
void get_refs(httplib::SSLClient& cli, unordered_set<string>& not_fetched, unordered_map<string,unordered_set<string>>& fetched_refs);