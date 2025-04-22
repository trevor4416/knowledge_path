#include <iostream>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "cpp-httplib/httplib.h"
#include "json/single_include/nlohmann/json.hpp"
#include "openalex.h"
#include "Graph.h"

using json = nlohmann::json;
using namespace std;

int main() {
    httplib::SSLClient cli("api.openalex.org",443);
    cli.enable_server_certificate_verification(true);
    // Graph test(cli, "https://doi.org/10.1126/scirobotics.aar3449", "https://doi.org/10.1089/soro.2013.0009"); // 1st level
    // Graph test(cli, "https://doi.org/10.1126/scirobotics.aar3449", "https://doi.org/10.1242/jeb.200.8.1165"); // 2nd level
    // Graph test(cli, "https://doi.org/10.1126/scirobotics.aar3449", "https://doi.org/10.1242/jeb.198.1.193"); // 3rd level
        // before excluding older nodes, Time elapsed: 90288 ms
        // after excluding older nodes, Time elapsed: 87875 ms
    // Graph test(cli, "https://doi.org/10.3390/app11114909", "https://doi.org/10.1089/soro.2013.0009"); // ?? -> 3rd level
        // before excluding older nodes, Time elapsed: 13526 ms
        // after excluding older nodes, Time elapsed: 10462 ms
    // verdict: requests take longer, but does reduce time overall by cutting down nodes traversed. not too significant in some cases
    return 0;
}