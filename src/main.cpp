#include <iostream>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "cpp-httplib/httplib.h"
#include "json/single_include/nlohmann/json.hpp"
#include "openalex.h"

using json = nlohmann::json;

int main() {

    /** Stuff for testing the openalex api **/
    // Create SSL client
    httplib::SSLClient cli("api.openalex.org",443);
    cli.enable_server_certificate_verification(true);

    json j = search_works(cli, "bioinspired robot", 3);
    // json j = get_work(cli, "https://doi.org/10.1088/1748-3182/7/2/025001");
    // json j = get_refs(cli, "https://doi.org/10.1088/1748-3182/7/2/025001");
    // json j = get_cites(cli, "https://openalex.org/W2078968880");

    // print response title, DOI

    if (j.is_object()) {
        std::string title = j.contains("title") && !j["title"].is_null() ? j["title"].get<std::string>() : "N/A";
        std::string doi = j.contains("doi") && !j["doi"].is_null() ? j["doi"].get<std::string>() : "N/A";
        std::string id = j.contains("id") && !j["id"].is_null() ? j["id"].get<std::string>() : "N/A";
        std::cout << "Title: " << title << "\n" << "DOI: " << doi << "\n" << "ID: " << id << "\n\n";
    }

    if (j.is_array()) {
        for (const auto& work : j) {
            // important to screen out works that lack titles, DOIs
            std::string title = work.contains("title") && !work["title"].is_null() ? work["title"].get<std::string>() : "N/A";
            std::string doi = work.contains("doi") && !work["doi"].is_null() ? work["doi"].get<std::string>() : "N/A";
            std::string id = work.contains("id") && !work["id"].is_null() ? work["id"].get<std::string>() : "N/A";
            std::cout << "Title: " << title << "\n" << "DOI: " << doi << "\n" << "ID: " << id << "\n\n";
        }
    }
    /** End of api testing stuff **/

    return 0;
}