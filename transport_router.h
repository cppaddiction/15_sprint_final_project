#pragma once
#include "router.h"
#include "transport_catalogue.h"
#include "json_builder.h"

struct EdgeWeight {
    constexpr EdgeWeight()=default;
    bool operator<(const EdgeWeight& second) const {return time_<second.time_;} 
    bool operator>(const EdgeWeight& second) const {return time_>second.time_;} 
    bool operator==(const EdgeWeight& second) const {return time_==second.time_;} 
    EdgeWeight operator+(const EdgeWeight& other) const {EdgeWeight ans; ans.time_ = time_ + other.time_; return ans;}
    double time_ = 0;
};

struct EdgeInfo {
    std::string bus_name;
    int span_count;
    double time;
    std::string from;
    std::string to;
};

class TransportRouter {
    public:
        TransportRouter(const transport_system::TransportCatalogue& tc);
        void SetBusWaitTime(int time);
        void SetBusVelocity(double bus_velocity);
        const graph::DirectedWeightedGraph<EdgeWeight>& MakeGraph();
        void GraphAddRoundTrip(const std::vector<transport_system::detail::StopData>& sps, const std::string& info);
        void GraphAddDefaultTrip(const std::vector<transport_system::detail::StopData>& sps, const std::string& info);
        json::Dict Build(const graph::Router<EdgeWeight>& rtr, const std::string& from, const std::string& to, int id) const;
    
    private:
        int bus_wait_time_;
        double bus_velocity_;
        const transport_system::TransportCatalogue& TC_;   
        const std::map<std::string, transport_system::detail::StopData>& stops_=TC_.GiveStopDataReadAccess();   
        const std::map<std::string, std::vector<transport_system::detail::StopData>>& bus_to_stops_ = TC_.GiveGraphInitHelpReadAccess();
        std::set<std::string> stops_by_index_;
        graph::DirectedWeightedGraph<EdgeWeight> graph;
        std::vector<EdgeInfo> vec_;
        const int m_in_km = 1000;
        const int m_in_hr = 60;
};