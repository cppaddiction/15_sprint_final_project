#include "json_reader.h"

void transport_system::JsonReader::LoadData(std::istream& in)
{
    auto ans=json::Load(in).GetRoot().AsMap();
    LoadStatRequests(ans["stat_requests"].AsArray());
}

void transport_system::JsonReader::LoadStatRequests(const json::Array& stat_requests)
{
    for(const auto& item: stat_requests)
    {
        const auto& m=item.AsMap();
        int x=m.at("id").AsInt();
        std::vector<std::string> y;
        if(m.at("type").IsString()&&m.at("type").AsString()=="Map")
        {
            TC_.PushRequestData(std::make_pair(x, y));
            continue;
        }
        else if(m.at("type").IsString()&&m.at("type").AsString()=="Route")
        {
            std::string from=m.at("from").AsString();
            std::string to=m.at("to").AsString();
            y.push_back(from);
            y.push_back(to);
            TC_.PushRequestData(std::make_pair(x, y));
            continue;
        }
        else //stop or bus
        {
            std::string s=m.at("type").AsString() + ' ' + m.at("name").AsString();
            y.push_back(s);
            TC_.PushRequestData(std::make_pair(x, y));
        }
    }
}
