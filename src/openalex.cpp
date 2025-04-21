#include "openalex.h"

json search_works(httplib::SSLClient& cli, const std::string& search_text, int num_results) {
    // create request string
    const std::string num_results_str = std::to_string(num_results);
    const std::string request_str = "/works?search=" + search_text + "&page=1&per-page=" + num_results_str;

    // GET request
    auto res = cli.Get(request_str); // GET request from works endpoint
    if (!res || res->status != 200) { // error message if response comes back unsuccessful or not at all
        std::cout << "Request Failed: " << (res ? std::to_string(res->status) : "no response") << std::endl;
        return {};
    }

    // parse response JSON
    json j = json::parse(res->body);

    return j["results"];
}

json get_work(httplib::SSLClient& cli, const std::string& id) {
    // create request string
    std::string request_str = "/works/" + id;

    // GET request
    httplib::Result res;
    do {
        res = cli.Get(request_str); // GET request from works endpoint
        if (!res || (res->status != 200 && (res->status < 300 || res->status >= 400))) { // error message if response comes back unsuccessful or not at all, and not a redirect
            std::cout << "Request Failed: " << (res ? std::to_string(res->status) : "no response") << std::endl;
            return {};
        } if (res->status != 200) { // if redirect
            request_str = res->get_header_value("Location");
        }
    } while (res->status >= 300 && res->status < 400); // if redirect

    // parse response JSON
    json j = json::parse(res->body);

    return j;
}

// TODO: rewrite because limited to 50 IDs per query
void get_refs(httplib::SSLClient& cli, unordered_set<string>& not_fetched, unordered_map<string,unordered_set<string>>& fetched_refs) {
    // create request string
    std::string request_str = "/works?filter=openalex_id:";

    for (const string& v : not_fetched)
        request_str += v + "|";
    request_str.erase(request_str.end-1);
    request_str += "&select=id,referenced_works";

    // GET request
    httplib::Result res = cli.Get(request_str); // GET request from works endpoint
    if (!res || res->status != 200) { // error message if response comes back unsuccessful or not at all
            if (res && res->status == 429) std::cout << "Rate Limited, Error: " << std::to_string(res->status) << std::endl;
            else std::cout << "Request Failed: " << (res ? std::to_string(res->status) : "no response") << std::endl;
            return;
    }

    // parse response JSON into array
    json j = json::parse(res->body)["results"];
    // objects in array include id and referenced_works, which itself is an array of openalex_ids
    for (const string& v : not_fetched) {

    }
}

vector<string> get_titles(httplib::SSLClient& cli,
                          const vector<string>& ids) {
    vector<string> titles;
    titles.reserve(ids.size());
    for (auto &id : ids) {
        auto w = get_work(cli, id);
        titles.push_back(w.value("title", "<no title>"));
    }
    return titles;
}