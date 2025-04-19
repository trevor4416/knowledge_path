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

json get_cites(httplib::SSLClient& cli, const std::string& id) {
    // create request string
    std::string request_str = "/works?filter=cites:" + id;

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

    return j["results"];
}

json get_refs(httplib::SSLClient& cli, const std::string& id) {
    json original = get_work(cli,id)["referenced_works"];
    // parse string ids
    std::vector<std::string> ids = original.get<std::vector<std::string>>();

    json j = json::array();
    for (const auto& id_str : ids)// get work json array from ids
        j.push_back(get_work(cli,id_str));

    return j;
}
