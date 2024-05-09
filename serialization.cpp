#include "serialization.h"

void Serializer::Serialize(std::istream& in)
{
    auto ans = json::Load(in).GetRoot().AsMap();
    LoadFileName(ans["serialization_settings"].AsMap());
    std::ofstream out(name_, std::ios::binary);
    SerializeBaseRequests(ans["base_requests"].AsArray(), out);
    SerializeRenderSettings(ans["render_settings"].AsMap(), out);
    SerializeRouterSettings(ans["routing_settings"].AsMap(), out);
    out.close();
}

void Serializer::Deserialize(transport_system::TransportCatalogue& TC_, map_entities::MapRenderer& MR_, TransportRouter& TR_)
{
    DeserializeBaseRequests(TC_);
    DeserializeRenderSettings(MR_);
    DeserializeRouterSettings(TR_);
}

void Serializer::SetFileName(const std::string& name) { name_ = name; }

void Serializer::LoadFileName(const json::Dict& serialization_settings)
{
    for (auto it = serialization_settings.begin(); it != serialization_settings.end(); it++)
    {
        if (it->first == "file")
        {
            SetFileName(it->second.AsString());
        }
    }
}

void Serializer::SerializeBaseRequests(const json::Array& base_requests, std::ostream& out)
{
    internal_stuff::TransportCatalogue result;

    SerializeStops(result, base_requests);
    SerializeBuses(result, base_requests);

    result.SerializeToOstream(&out);
}

void Serializer::SerializeStops(internal_stuff::TransportCatalogue& result, const json::Array& base_requests)
{
    for (const auto& item : base_requests)
    {
        const auto& m = item.AsMap();
        if (m.at("type").IsString() && m.at("type").AsString() == "Stop" && m.at("name").IsString())
        {
            internal_stuff::Stop s;
            s.set_lat(m.at("latitude").AsDouble());
            s.set_lng(m.at("longitude").AsDouble());
            s.set_name(m.at("name").AsString());
            const auto& stops = m.at("road_distances").AsMap();
            if (stops.size() > 0)
            {
                for (auto it = stops.begin(); it != stops.end(); it++)
                {
                    internal_stuff::Connection conn;
                    conn.set_to(it->first);
                    conn.set_distance(stops.at(it->first).AsDouble());
                    *s.add_connections() = conn;
                }
            }
            s.set_type("Stop");
            *result.add_stops() = s;
        }
    }
}

void Serializer::SerializeBuses(internal_stuff::TransportCatalogue& result, const json::Array& base_requests)
{
    for (const auto& item : base_requests)
    {
        const auto& m = item.AsMap();
        if (m.at("type").IsString() && m.at("type").AsString() == "Bus" && m.at("name").IsString())
        {
            internal_stuff::Bus b;
            b.set_isroundtrip(m.at("is_roundtrip").AsBool());
            b.set_name(m.at("name").AsString());
            const auto& stops = m.at("stops").AsArray();
            for (const auto& stop : stops)
            {
                if (stop.IsString())
                {
                    *b.add_sps() = stop.AsString();
                }
            }
            if (m.at("is_roundtrip").AsBool())
            {
                *b.add_sps() = ">";
            }
            else
            {
                *b.add_sps() = "-";
            }
            b.set_type("Bus");
            *result.add_buses() = b;
        }
    }
}

void Serializer::SerializeRenderSettings(const json::Dict& render_settings, std::ostream& out)
{
    internal_stuff::RenderSettings result;

    for (auto it = render_settings.begin(); it != render_settings.end(); it++)
    {
        if (it->first == "width")
        {
            result.set_w(it->second.AsDouble());
            continue;
        }
        else if (it->first == "height")
        {
            result.set_h(it->second.AsDouble());
            continue;
        }
        else if (it->first == "padding")
        {
            result.set_pad(it->second.AsDouble());
            continue;
        }
        else if (it->first == "line_width")
        {
            result.set_line_w(it->second.AsDouble());
            continue;
        }
        else if (it->first == "stop_radius")
        {
            result.set_stop_r(it->second.AsDouble());
            continue;
        }
        else if (it->first == "bus_label_font_size")
        {
            result.set_bus_l_f_size(it->second.AsInt());
            continue;
        }
        else if (it->first == "bus_label_offset")
        {
            const auto& x = it->second.AsArray();
            result.mutable_bus_l_offset()->set_dx(x[0].AsDouble());
            result.mutable_bus_l_offset()->set_dy(x[1].AsDouble());
            continue;
        }
        else if (it->first == "stop_label_font_size")
        {
            result.set_stop_l_f_size(it->second.AsInt());
            continue;
        }
        else if (it->first == "stop_label_offset")
        {
            const auto& x = it->second.AsArray();
            result.mutable_stop_l_offset()->set_dx(x[0].AsDouble());
            result.mutable_stop_l_offset()->set_dy(x[1].AsDouble());
            continue;
        }
        else if (it->first == "underlayer_color")
        {
            ProcessSingleColor(result, it->second);
            continue;
        }
        else if (it->first == "underlayer_width")
        {
            result.set_underlayer_w(it->second.AsDouble());
            continue;
        }
        else
        {
            ProcessColorArray(result, it->second.AsArray());
        }
    }
    result.SerializeToOstream(&out);
}

void Serializer::ProcessSingleColor(internal_stuff::RenderSettings& result, const json::Node& item)
{
    if (item.IsString())
    {
        result.mutable_underlayer_col()->set_is_string(true);
        result.mutable_underlayer_col()->set_str_color(item.AsString());
    }
    else
    {
        const auto& x = item.AsArray();
        if (x.size() == 3)
        {
            result.mutable_underlayer_col()->set_is_color_rgb(true);
            auto rgb_pointer = result.mutable_underlayer_col()->mutable_rgb_color();
            rgb_pointer->set_r(x[0].AsInt());
            rgb_pointer->set_g(x[1].AsInt());
            rgb_pointer->set_b(x[2].AsInt());
        }
        else
        {
            result.mutable_underlayer_col()->set_is_color_rgba(true);
            auto rgba_pointer = result.mutable_underlayer_col()->mutable_rgba_color();
            rgba_pointer->set_r(x[0].AsInt());
            rgba_pointer->set_g(x[1].AsInt());
            rgba_pointer->set_b(x[2].AsInt());
            rgba_pointer->set_opac(x[3].AsDouble());
        }
    }
}

void Serializer::ProcessColorArray(internal_stuff::RenderSettings& result, const json::Array& x)
{
    for (const auto& item : x)
    {
        internal_stuff::Color color;
        if (item.IsString())
        {
            color.set_is_string(true);
            color.set_str_color(item.AsString());
        }
        else
        {
            auto y = item.AsArray();
            if (y.size() == 3)
            {
                color.set_is_color_rgb(true);
                auto rgb_pointer = color.mutable_rgb_color();
                rgb_pointer->set_r(y[0].AsInt());
                rgb_pointer->set_g(y[1].AsInt());
                rgb_pointer->set_b(y[2].AsInt());
            }
            else
            {
                color.set_is_color_rgba(true);
                auto rgba_pointer = color.mutable_rgba_color();
                rgba_pointer->set_r(y[0].AsInt());
                rgba_pointer->set_g(y[1].AsInt());
                rgba_pointer->set_b(y[2].AsInt());
                rgba_pointer->set_opac(y[3].AsDouble());
            }
        }
        *result.add_color_palet() = color;
    }
}

void Serializer::SerializeRouterSettings(const json::Dict& router_settings, std::ostream& out)
{
    internal_stuff::RouterSettings result;
    for (auto it = router_settings.begin(); it != router_settings.end(); it++)
    {
        if (it->first == "bus_velocity")
        {
            result.set_b_velo(it->second.AsDouble());
        }
        else
        {
            result.set_b_w_time(it->second.AsInt());
        }
    }
    result.SerializeToOstream(&out);
}

void Serializer::DeserializeBaseRequests(transport_system::TransportCatalogue& TC_)
{
    std::ifstream in(name_, std::ios::binary);
    internal_stuff::TransportCatalogue cat;
    cat.ParseFromIstream(&in);

    DeserializeStops(TC_, cat);
    DeserializeBuses(TC_, cat);
}

void Serializer::DeserializeStops(transport_system::TransportCatalogue& TC_, const internal_stuff::TransportCatalogue& cat)
{
    for (const auto& item : cat.stops())
    {
        transport_system::detail::StopData result;
        result.name = item.name();
        result.coords.lat = item.lat();
        result.coords.lng = item.lng();

        if (item.connections_size() > 0)
        {
            for (const auto& connec : item.connections())
            {
                result.connections[connec.to()] = connec.distance();
            }
        }

        TC_.PushStopData(result.name, result);
        TC_.PushBusThroughStopData(result.name);
    }
}

void Serializer::DeserializeBuses(transport_system::TransportCatalogue& TC_, const internal_stuff::TransportCatalogue& cat)
{
    for (const auto& item : cat.buses())
    {
        std::vector<std::string> r;
        std::vector<transport_system::detail::StopData> v;
        for (const auto& stop : item.sps())
        {
            r.push_back(stop);
            for (const auto& i : cat.stops())
            {
                if (i.name() == stop)
                {
                    transport_system::detail::StopData result;
                    result.name = i.name();
                    result.coords.lat = i.lat();
                    result.coords.lng = i.lng();

                    if (i.connections_size() > 0)
                    {
                        for (const auto& connec : i.connections())
                        {
                            result.connections[connec.to()] = connec.distance();
                        }
                    }
                    v.push_back(result);
                }
            }
        }
        if (item.isroundtrip())
        {
            v.push_back(transport_system::detail::StopData{ ">" });
        }
        else
        {
            v.push_back(transport_system::detail::StopData{ "-" });
        }

        TC_.PushPathData("Bus " + item.name(), r);
        for (int i = 0; i < static_cast<int>(r.size()) - 1; i++)
        {
            TC_.PushBusThroughStopData(r[i], "Bus " + item.name());
        }
        TC_.PushGraphInitHelpData("Bus " + item.name(), v);
    }
}

void Serializer::DeserializeRenderSettings(map_entities::MapRenderer& MR_)
{
    std::ifstream in(name_, std::ios::binary);
    internal_stuff::RenderSettings rs;
    rs.ParseFromIstream(&in);

    MR_.SetWidth(rs.w());
    MR_.SetHeight(rs.h());
    MR_.SetPadding(rs.pad());
    MR_.SetLineWidth(rs.line_w());
    MR_.SetStopRadius(rs.stop_r());
    MR_.SetBusLabelFontSize(rs.bus_l_f_size());
    MR_.SetBusLabelOffsetX(rs.bus_l_offset().dx());
    MR_.SetBusLabelOffsetY(rs.bus_l_offset().dy());
    MR_.SetStopLabelFontSize(rs.stop_l_f_size());
    MR_.SetStopLabelOffsetX(rs.stop_l_offset().dx());
    MR_.SetStopLabelOffsetY(rs.stop_l_offset().dy());

    svg::Color underlayer_color = svg::Rgb(0, 0, 0);
    if (rs.underlayer_col().is_string())
    {
        MR_.SetUnderlayerColor(rs.underlayer_col().str_color());
    }
    else if (rs.underlayer_col().is_color_rgb())
    {
        MR_.SetUnderlayerColor(svg::Rgb(rs.underlayer_col().rgb_color().r(), rs.underlayer_col().rgb_color().g(), rs.underlayer_col().rgb_color().b()));
    }
    else if (rs.underlayer_col().is_color_rgba())
    {
        MR_.SetUnderlayerColor(svg::Rgba(rs.underlayer_col().rgba_color().r(), rs.underlayer_col().rgba_color().g(), rs.underlayer_col().rgba_color().b(), rs.underlayer_col().rgba_color().opac()));
    }
    else
    {
        MR_.SetUnderlayerColor(underlayer_color);
    }

    MR_.SetUnderlayerWidth(rs.underlayer_w());

    std::vector<svg::Color> color_palette = {};
    for (const auto& col : rs.color_palet())
    {
        if (col.is_string())
        {
            color_palette.emplace_back(col.str_color());
        }
        else
        {
            if (col.is_color_rgb())
            {
                color_palette.emplace_back(svg::Rgb(col.rgb_color().r(), col.rgb_color().g(), col.rgb_color().b()));
            }
            else
            {
                color_palette.emplace_back(svg::Rgba(col.rgba_color().r(), col.rgba_color().g(), col.rgba_color().b(), col.rgba_color().opac()));
            }
        }
    }
    MR_.SetColorPalette(color_palette);
}

void Serializer::DeserializeRouterSettings(TransportRouter& TR_)
{
    std::ifstream in(name_, std::ios::binary);
    internal_stuff::RouterSettings r_s;
    r_s.ParseFromIstream(&in);

    TR_.SetBusVelocity(r_s.b_velo());
    TR_.SetBusWaitTime(r_s.b_w_time());
}