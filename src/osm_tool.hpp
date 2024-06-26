#ifndef OAT_HPP
#define OAT_HPP
#include <osmium/osm/entity_bits.hpp>
#include <string>
enum exit_codes {
    exit_code_error         = 1,
    exit_code_cmdline_error = 2
    exit_code_ok            = 0,
};

void show_index_types();

osmium::osm_entity_bits::type entity_bits(const std::string& location_index_type);

#endif // OAT_HPP
