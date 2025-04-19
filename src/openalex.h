#ifndef OPENALEX_H
#define OPENALEX_H

#endif //OPENALEX_H

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "cpp-httplib/httplib.h"
#include "json/single_include/nlohmann/json.hpp"

using json = nlohmann::json;

// declaration for helper functions to get data from api.openalex.org

// these helper functions take an httplib::SSLClient as a parameter,
// so that a new client need not be created for every function call
// all return either a json work object or a json array of work objects

// searches for search_text in titles, abstracts, and fulltext, and returns the first num_results results
json search_works(httplib::SSLClient& cli, const std::string& search_text, int num_results);

// returns the work specified by the openalex ID, the DOI, or other supported external IDs (https://docs.openalex.org/api-entities/works/get-a-single-work)
json get_work(httplib::SSLClient& cli, const std::string& id);

// gets the works that cite this one; these are the incoming edges of the citation graph; id MUST BE OpenAlex ID
json get_cites(httplib::SSLClient& cli, const std::string& id);

// gets the works referenced by this one; these are the outgoing edges of the citation graph
json get_refs(httplib::SSLClient& cli, const std::string& id);