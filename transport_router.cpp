#include "transport_router.h"

TransportRouter::TransportRouter(const transport_system::TransportCatalogue& tc) : TC_(tc) {}

void TransportRouter::SetBusWaitTime(int time) { bus_wait_time_ = time; }

void TransportRouter::SetBusVelocity(double bus_velocity) { bus_velocity_ = bus_velocity; }

const graph::DirectedWeightedGraph<EdgeWeight>& TransportRouter::MakeGraph()
{
    for (auto it = stops_.begin(); it != stops_.end(); it++)
    {
        stops_by_index_.insert(it->first);
    }

    graph.init(stops_by_index_.size());
    for (auto it = bus_to_stops_.begin(); it != bus_to_stops_.end(); it++)
    {
        const auto& sps = it->second;
        if (sps.size() > 0)
        {
            if (sps[0].name==sps[sps.size()-2].name)
            {
                GraphAddRoundTrip(sps, it->first.substr(4, it->first.size() - 4));
            }
            else
            {
                GraphAddDefaultTrip(sps, it->first.substr(4, it->first.size() - 4));
            }
        }
    }
    return graph;
}

void TransportRouter::GraphAddRoundTrip(const std::vector<transport_system::detail::StopData>& sps, const std::string& info)
{
    int offset = 1;
    for (auto itt = sps.begin(); itt != sps.end() - 2; itt++)
    {
        double delta = 0;
        auto stop1 = *itt;
        for (auto ittt = std::next(sps.begin(), offset); ittt != sps.end() - 1; ittt++)
        {
            auto stop2 = *ittt;
            try
            {
                delta += stop1.connections.at(stop2.name);
            }
            catch (...)
            {
                delta += stop2.connections.at((*next(ittt, -1)).name);
            }
            stop1 = stop2;
            double time_required = delta / m_in_km / bus_velocity_ * m_in_hr + bus_wait_time_;

            graph::Edge<EdgeWeight> bar;
            bar.from = static_cast<size_t>(std::distance(stops_by_index_.begin(), stops_by_index_.find((*itt).name)));
            bar.to = static_cast<size_t>(std::distance(stops_by_index_.begin(), stops_by_index_.find((*ittt).name)));
            EdgeWeight ew{};
            ew.time_ = time_required;
            bar.weight = ew;

            graph.AddEdge(bar);
            vec_.push_back(EdgeInfo{info, static_cast<int>(std::distance(itt, ittt)), delta / 1000 / bus_velocity_ * 60, (*itt).name , (*ittt).name });
        }
        offset += 1;
    }
}

void TransportRouter::GraphAddDefaultTrip(const std::vector<transport_system::detail::StopData>& sps, const std::string& info)
{
    int offset = -3;
    for (int k = sps.size() - 2; k > 0; k--)
    {
        double delta_forw = 0;
        double delta_backw = 0;
        auto stop1 = sps[k];
        for (int i = sps.size() + offset; i >= 0; i--)
        {
            auto stop2 = sps[i];
            bool forward_path_avail = false;
            bool backward_path_avail = false;
            double dist_forw = 0;
            double dist_backw = 0;
            try
            {
                dist_forw = stop1.connections.at(stop2.name);
                forward_path_avail = true;
            }
            catch (...)
            {

            }
            try
            {
                dist_backw = stop2.connections.at(sps[i + 1].name);
                backward_path_avail = true;
            }
            catch (...)
            {

            }
            if (forward_path_avail && backward_path_avail)
            {
                delta_forw += dist_forw;
                delta_backw += dist_backw;
                double time_required_forw = delta_forw / m_in_km / bus_velocity_ * m_in_hr + bus_wait_time_;
                double time_required_backw = delta_backw / m_in_km / bus_velocity_ * m_in_hr + bus_wait_time_;

                graph::Edge<EdgeWeight> bar_forw;
                bar_forw.from = static_cast<size_t>(std::distance(stops_by_index_.begin(), stops_by_index_.find(stop1.name)));
                bar_forw.to = static_cast<size_t>(std::distance(stops_by_index_.begin(), stops_by_index_.find(stop2.name)));
                EdgeWeight ew_forw{};
                ew_forw.time_ = time_required_forw;
                bar_forw.weight = ew_forw;

                graph::Edge<EdgeWeight> bar_backw;
                bar_backw.from = bar_forw.to;
                bar_backw.to = bar_forw.from;
                EdgeWeight ew_backw{};
                ew_backw.time_ = time_required_backw;
                bar_backw.weight = ew_backw;

                graph.AddEdge(bar_forw);
                graph.AddEdge(bar_backw);
                vec_.push_back(EdgeInfo{ info, std::abs(i - k), dist_forw / m_in_km / bus_velocity_ * m_in_hr, stop1.name , stop2.name });
                vec_.push_back(EdgeInfo{ info, std::abs(i - k), dist_backw / m_in_km / bus_velocity_ * m_in_hr, stop2.name , stop1.name });
            }
            else
            {
                double dist;
                if (forward_path_avail)
                {
                    dist = dist_forw;
                }
                else
                {
                    dist = dist_backw;
                }

                delta_forw += dist;
                delta_backw += dist;
                double time_required_forw = delta_forw / m_in_km / bus_velocity_ * m_in_hr + bus_wait_time_;
                double time_required_backw = delta_backw / m_in_km / bus_velocity_ * m_in_hr + bus_wait_time_;

                graph::Edge<EdgeWeight> bar_forw;
                bar_forw.from = static_cast<size_t>(std::distance(stops_by_index_.begin(), stops_by_index_.find(stop1.name)));
                bar_forw.to = static_cast<size_t>(std::distance(stops_by_index_.begin(), stops_by_index_.find(stop2.name)));
                EdgeWeight ew_forw{};
                ew_forw.time_ = time_required_forw;
                bar_forw.weight = ew_forw;

                graph::Edge<EdgeWeight> bar_backw;
                bar_backw.from = bar_forw.to;
                bar_backw.to = bar_forw.from;
                EdgeWeight ew_backw{};
                ew_backw.time_ = time_required_backw;
                bar_backw.weight = ew_backw;

                graph.AddEdge(bar_forw);
                graph.AddEdge(bar_backw);
                vec_.push_back(EdgeInfo{ info, std::abs(i - k), dist_forw / m_in_km / bus_velocity_ * m_in_hr, stop1.name, stop2.name });
                vec_.push_back(EdgeInfo{ info, std::abs(i - k), dist_backw / m_in_km / bus_velocity_ * m_in_hr, stop2.name, stop1.name });

            }
            stop1 = stop2;
        }
        offset -= 1;
    }
}

json::Dict TransportRouter::Build(const graph::Router<EdgeWeight>& rtr, const std::string& from, const std::string& to, int id) const
{
    json::Builder result;
    result.StartDict().Key("request_id").Value(id);

    auto from_ = static_cast<size_t>(distance(stops_by_index_.begin(), stops_by_index_.find(from)));
    auto to_ = static_cast<size_t>(distance(stops_by_index_.begin(), stops_by_index_.find(to)));

    auto response = rtr.BuildRoute(from_, to_);
    if (response)
    {
        result.Key("total_time").Value((*response).weight.time_);
        result.Key("items").StartArray();
        auto edges = (*response).edges;
        for (int i = 0; i < edges.size(); i++)
        {
            auto info = vec_[edges[i]];
            const std::string wait = "Wait";
            const std::string bus = "Bus";
            result.StartDict().Key("stop_name").Value(info.from).Key("time").Value(bus_wait_time_).Key("type").Value(wait).EndDict();
            result.StartDict().Key("bus").Value(info.bus_name).Key("span_count").Value(info.span_count).Key("time").Value(info.time).Key("type").Value(bus).EndDict();
        }
        result.EndArray();
    }
    else
    {
        const std::string not_found = "not found";
        result.Key("error_message").Value(not_found);
    }
    return result.EndDict().Build().AsMap();
}