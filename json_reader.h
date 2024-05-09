#pragma once 
#include "json.h" 
#include "map_renderer.h" 
#include "transport_router.h" 
#include <fstream> 
#include <transport_catalogue.pb.h>

namespace transport_system {

    class JsonReader {
        public:
            explicit JsonReader(TransportCatalogue& tc) : TC_(tc) {}
            void LoadData(std::istream& in);
            void LoadStatRequests(const json::Array& stat_requests);

        private:
            TransportCatalogue& TC_;
    };

}
