#include <fstream>
#include <iostream>
#include <string_view>

#include "request_handler.h"
#include "serialization.h"

using namespace std::literals;

using namespace std;
using namespace map_entities;
using namespace transport_system;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    TransportCatalogue catalogue;
    MapRenderer renderer(catalogue);
    TransportRouter router(catalogue);
    JsonReader j_reader(catalogue);
    RequestHandler r_handler(catalogue, renderer, router);
    Serializer serializer(catalogue);

    if (mode == "make_base"sv)
    {
        serializer.Serialize(cin);
    }
    else if (mode == "process_requests"sv) {
        serializer.Deserialize(catalogue, renderer, router);
        j_reader.LoadData(cin);

        graph::Router<EdgeWeight> router_(router.MakeGraph());

        r_handler.FormAnswer(router_, cout);
    }
    else {
        PrintUsage();
        return 1;
    }
}