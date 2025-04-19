#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <sstream>
#include <string>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "cpp-httplib/httplib.h"
#include "json/single_include/nlohmann/json.hpp"
#include "openalex.h"

using json = nlohmann::json;

TEST_CASE("Search Test", "[tag]") {
    std::string expected = R"(Title: Materials, Actuators, and Sensors for Soft Bioinspired Robots
DOI: https://doi.org/10.1002/adma.202003139
ID: https://openalex.org/W3117219792

Title: Soft robotics: a bioinspired evolution in robotics
DOI: https://doi.org/10.1016/j.tibtech.2013.03.002
ID: https://openalex.org/W2079574144

Title: Design and fabrication of multi-material structures for bioinspired robots
DOI: https://doi.org/10.1098/rsta.2009.0013
ID: https://openalex.org/W2114434484

)";
    // Create SSL client
    httplib::SSLClient cli("api.openalex.org",443);
    cli.enable_server_certificate_verification(true);

    json j = search_works(cli, "bioinspired robot", 3);
    // json j = get_work(cli, "https://doi.org/10.1088/1748-3182/7/2/025001");
    // json j = get_refs(cli, "https://doi.org/10.1088/1748-3182/7/2/025001");
    // json j = get_cites(cli, "https://openalex.org/W2078968880");

    std::ostringstream oss;
    if (j.is_object()) {
        std::string title = j.contains("title") && !j["title"].is_null() ? j["title"].get<std::string>() : "N/A";
        std::string doi = j.contains("doi") && !j["doi"].is_null() ? j["doi"].get<std::string>() : "N/A";
        std::string id = j.contains("id") && !j["id"].is_null() ? j["id"].get<std::string>() : "N/A";

        oss << "Title: " << title << "\n"
            << "DOI: " << doi << "\n"
            << "ID: " << id << "\n\n";
    }

    if (j.is_array()) {
        for (const auto& work : j) {
            std::string title = work.contains("title") && !work["title"].is_null() ? work["title"].get<std::string>() : "N/A";
            std::string doi = work.contains("doi") && !work["doi"].is_null() ? work["doi"].get<std::string>() : "N/A";
            std::string id = work.contains("id") && !work["id"].is_null() ? work["id"].get<std::string>() : "N/A";

            oss << "Title: " << title << "\n"
                << "DOI: " << doi << "\n"
                << "ID: " << id << "\n\n";
        }
    }

    REQUIRE(expected == oss.str());
}