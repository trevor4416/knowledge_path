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

void get_refs(httplib::SSLClient& cli, unordered_set<string>& not_fetched, unordered_map<string,unordered_set<string>>& fetched_refs, int year_target) {
    while (!not_fetched.empty()) {
        cout << "Fetching (" << to_string(not_fetched.size()) << "->" << to_string(fetched_refs.size()) << ")" << endl;
        // create request string
        string request_str = "/works?filter=openalex_id:";
        int it = 0;
        for (const string& v : not_fetched) {
            request_str += v + "|";
            if (it == 49) break;
            it++;
        }
        request_str.erase(request_str.length()-1);
        request_str += "&select=id,publication_year,referenced_works";

        // GET request
        httplib::Result res = cli.Get(request_str); // GET request from works endpoint
        if (!res || res->status != 200) { // error message if response comes back unsuccessful or not at all
            if (res && res->status == 429) std::cout << "Rate Limited! Code: " << std::to_string(res->status) << std::endl;
            else std::cout << "Request Failed: " << (res ? std::to_string(res->status) : "no response") << std::endl;
            return;
        }
        // cout << "Request success!" << endl;
        // cout << request_str << endl;

        // parse response JSON into array
        json j = json::parse(res->body)["results"];
        // objects in array include id and referenced_works, which itself is an array of openalex_ids

        for (size_t i = 0; i < j.size(); i++) {
            // cout << "ID: " << j[i]["id"] << "; Title: " << j[i]["title"] << endl;
            if (j[i].value("publication_year",0) >= year_target) { // only fetch references for works that aren't older than the target
                for (const auto& ref : j[i]["referenced_works"]) {
                    fetched_refs[j[i]["id"]].emplace(ref);
                }
            }
            not_fetched.erase(j[i]["id"]);
        }
        // some papers are available through a work/ request and not a works? request
        // these seem to have old, conflicting IDs associated
        // cout << j.size() << endl;
        if (j.size() == 0 && not_fetched.size() != 0) {
            for (const string& single_ref : not_fetched) {
                cout << "Single Fetch!" << endl;
                json j_single = get_work(cli,single_ref); // TODO: optimize for id, publication_year, referenced_works only
                // cout << "ID: " << j_single["id"] << endl;
                // cout << "not fetched ID: " << single_ref << endl;
                if (!j_single.is_null() && j_single.value("publication_year",0) >= year_target) { // only fetch references for works that aren't older than the target
                    for (const auto& ref : j_single["referenced_works"]) {
                        fetched_refs[single_ref].emplace(ref);
                    }
                }
                not_fetched.erase(single_ref);
            }
        }
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