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

    return 0;
}