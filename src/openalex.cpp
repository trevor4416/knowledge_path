#include "openalex.h"

// Search works by text.
json search_works(httplib::SSLClient& cli,
                  const std::string& search_text,
                  int num_results) {
    const std::string request_str = "/works?search=" + search_text +
                                    "&page=1&per-page=" +
                                    std::to_string(num_results);
    auto res = cli.Get(request_str);
    if (!res || res->status != 200) {
        std::cout << "Request failed: "
                  << (res ? std::to_string(res->status) : "no response")
                  << std::endl;
        return {};
    }
    json j = json::parse(res->body);
    return j["results"];
}

// Retrieve a work by ID, following redirects.
json get_work(httplib::SSLClient& cli,
              const std::string& id) {
    std::string request_str = "/works/" + id;
    httplib::Result res;
    do {
        res = cli.Get(request_str);
        if (!res || (res->status != 200 && (res->status < 300 || res->status >= 400))) {
            std::cout << "Request failed: "
                      << (res ? std::to_string(res->status) : "no response")
                      << std::endl;
            return {};
        }
        if (res->status != 200) {
            request_str = res->get_header_value("Location");
        }
    } while (res->status >= 300 && res->status < 400);

    return json::parse(res->body);
}

// Fetch references for works until none remain.
void get_refs(httplib::SSLClient& cli,
              std::unordered_set<std::string>& not_fetched,
              std::unordered_map<std::string, std::unordered_set<std::string>>& fetched_refs,
              int year_target) {
    while (!not_fetched.empty()) {
        std::cout << "Fetching (" << not_fetched.size() << " -> "
                  << fetched_refs.size() << ")" << std::endl;

        std::string request_str = "/works?filter=openalex_id:";
        int count = 0;
        for (const auto& id : not_fetched) {
            request_str += id + "|";
            if (++count == 50) break;
        }

        auto res = cli.Get(request_str);
        if (!res || res->status != 200) {
            if (res && res->status == 429)
                std::cout << "Rate limited: " << res->status << std::endl;
            else
                std::cout << "Request failed: "
                          << (res ? std::to_string(res->status) : "no response")
                          << std::endl;
            return;
        }

        json j = json::parse(res->body)["results"];
        for (const auto& item : j) {
            if (item.value("publication_year", 0) >= year_target) {
                for (const auto& ref : item["referenced_works"]) {
                    fetched_refs[item["id"]].emplace(ref);
                }
            }
            not_fetched.erase(item["id"]);
        }

        if (j.empty() && !not_fetched.empty()) {
            for (auto it = not_fetched.begin(); it != not_fetched.end(); ) {
                json single = get_work(cli, *it);
                if (!single.is_null() && single.value("publication_year", 0) >= year_target) {
                    for (const auto& ref : single["referenced_works"]) {
                        fetched_refs[*it].emplace(ref);
                    }
                }
                it = not_fetched.erase(it);
            }
        }
    }
}

// Get titles for a list of work IDs.
std::vector<std::string> get_titles(httplib::SSLClient& cli,
                                    const std::vector<std::string>& ids) {
    std::vector<std::string> titles;
    titles.reserve(ids.size());
    for (const auto& id : ids) {
        auto w = get_work(cli, id);
        titles.push_back(w.value("title", "<no title>"));
    }
    return titles;
}

// Compute concept overlap score.
float get_heuristic(const json& concepts,
                    size_t /*unused*/,
                    const std::unordered_map<std::string, float>& target) {
    float min_sum = 0.0f, max_sum = 0.0f;
    for (const auto& c : concepts) {
        float score = c.value("score", 0.0f);
        auto it = target.find(c["id"]);
        if (it != target.end()) {
            min_sum += std::min(it->second, score);
            max_sum += std::max(it->second, score);
        } else {
            max_sum += score;
        }
    }
    return min_sum / max_sum;
}

// Retrieve references prioritized by heuristic.
std::vector<std::pair<float, std::string>> get_refs_befs(
    httplib::SSLClient& cli,
    const std::string& id,
    const std::unordered_map<std::string, float>& target_concepts,
    int year_target) {
    std::vector<std::pair<float, std::string>> result;
    std::string request_str = "/works/" + id.substr(21) + "?select=referenced_works";
    auto res = cli.Get(request_str);
    if (!res || res->status != 200) return {};

    json refs = json::parse(res->body)["referenced_works"];
    size_t idx = 0;
    while (idx < refs.size()) {
        request_str = "/works?filter=openalex_id:";
        for (int i = 0; i < 50 && idx < refs.size(); ++i, ++idx) {
            request_str += refs[idx].get<std::string>() + "|";
        }
        request_str.pop_back();
        request_str += "&select=id,publication_year,referenced_works,concepts";

        res = cli.Get(request_str);
        if (res && res->status == 200) {
            for (const auto& w : json::parse(res->body)["results"]) {
                if (w.value("publication_year", 0) >= year_target) {
                    float h = get_heuristic(
                        w["concepts"],
                        w["referenced_works"].size(),
                        target_concepts
                    );
                    result.emplace_back(h, w["id"]);
                }
            }
        }
    }
    return result;
}
