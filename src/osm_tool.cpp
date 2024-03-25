#include "osm_tool.hpp"

#include <osmium/index/map.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/osm/types.hpp>

#include <iostream>

using namespace std;

osmium::osm_entity_bits::type entity_bits(const string& index_type) {
    if (index_type == "none")
        return osmium::osm_entity_bits::way;
    return osmium::osm_entity_bits::way | osmium::osm_entity_bits::node;
}

void show_index_types() {
    const auto& index_factory = osmium::index::MapFactory<osmium::unsigned_object_id_type, osmium::Location>::instance();
    for (const auto& type : index_factory.map_types())
        cout << "  " << type << "\n";
}

