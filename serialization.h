#pragma once
#include "json.h"
#include <fstream>
#include <transport_catalogue.pb.h>
#include "transport_router.h"
#include "map_renderer.h"

class Serializer {
public:
    Serializer(const transport_system::TransportCatalogue& tc) : TC_(tc) {}
    void Serialize(std::istream& in);
    void Deserialize(transport_system::TransportCatalogue& TC_, map_entities::MapRenderer& MR_, TransportRouter& TR_);
    void SetFileName(const std::string& name);
    void LoadFileName(const json::Dict& serialization_settings);
    void SerializeBaseRequests(const json::Array& base_requests, std::ostream& out);
    void SerializeRenderSettings(const json::Dict& render_settings, std::ostream& out);
    void SerializeRouterSettings(const json::Dict& router_settings, std::ostream& out);
    void DeserializeBaseRequests(transport_system::TransportCatalogue& TC_);
    void DeserializeRenderSettings(map_entities::MapRenderer& MR_);
    void DeserializeRouterSettings(TransportRouter& TR_);
    void ProcessSingleColor(internal_stuff::RenderSettings& result, const json::Node& item);
    void ProcessColorArray(internal_stuff::RenderSettings& result, const json::Array& x);
    void SerializeStops(internal_stuff::TransportCatalogue& result, const json::Array& base_requests);
    void SerializeBuses(internal_stuff::TransportCatalogue& result, const json::Array& base_requests);
    void DeserializeStops(transport_system::TransportCatalogue& TC_, const internal_stuff::TransportCatalogue& cat);
    void DeserializeBuses(transport_system::TransportCatalogue& TC_, const internal_stuff::TransportCatalogue& cat);

private:
    const transport_system::TransportCatalogue& TC_;
    std::string name_;
};