#include "osm_tool.hpp"

#ifdef WITH_OLD_STYLE_MP_SUPPORT
# include <osmium/area/assembler_legacy.hpp>
# include <osmium/area/multipolygon_manager_legacy.hpp>
#else
# include <osmium/area/assembler.hpp>
# include <osmium/area/multipolygon_manager.hpp>
#endif

#include <osmium/area/problem_reporter_ogr.hpp>
#include <osmium/geom/ogr.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/index/map/dense_mem_array.hpp>
#include <osmium/index/map/dense_mmap_array.hpp>
#include <osmium/index/map/dummy.hpp>
#include <osmium/index/map/flex_mem.hpp>
#include <osmium/index/map/sparse_mem_array.hpp>
#include <osmium/index/map/sparse_mmap_array.hpp>
#include <osmium/index/node_locations_map.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>
#include <osmium/util/memory.hpp>
#include <osmium/util/verbose_output.hpp>
#include <osmium/visitor.hpp>

#include <cstdlib>
#include <getopt.h>
#include <iostream>

using namespace std;

using index_type = osmium::index::map::Map<osmium::unsigned_object_id_type, osmium::Location>;
using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;

REGISTER_MAP(osmium::unsigned_object_id_type, osmium::Location, osmium::index::map::Dummy, none)

void print_help() {
    cout << "oat_failed_area_tags [OPTIONS] OSMFILE\n\n"
              << "Build areas from OSMFILE and count tags where area assembly failed.\n\n"
              << "Options:\n"
              << "  -i, --index=INDEX_TYPE       Set index type for location index (default: flex_mem)\n"
              << "  -h, --help                   This help message\n"
              << "  -I, --show-index-types       Show available index types for location index\n"
              ;
}

#ifdef WITH_OLD_STYLE_MP_SUPPORT
using assembler_type = osmium::area::AssemblerLegacy;
using mp_manager_type = osmium::area::MultipolygonManagerLegacy<assembler_type>;
#else
using assembler_type = osmium::area::Assembler;
using mp_manager_type = osmium::area::MultipolygonManager<assembler_type>;
#endif

struct tag_counter {
    size_t landuse  = 0;
    size_t amenity  = 0;
    size_t building = 0;
    size_t sport    = 0;
    size_t boundary = 0;
    size_t leisure  = 0;
    size_t place    = 0;
    size_t name     = 0;
    size_t waterway = 0;
    size_t natural  = 0;
    size_t unknown  = 0;
};

int main(int argc, char* argv[]) {
    try {
        ios_base::sync_with_stdio(false);

        static const struct option long_options[] = {
            {"help",       no_argument,       nullptr, 'h'},
            {"index",      required_argument, nullptr, 'i'},
            {"show-index", no_argument,       nullptr, 'I'},
            {nullptr, 0, nullptr, 0}
        };

        string location_index_type{"flex_mem"};
        const auto& map_factory = osmium::index::MapFactory<osmium::unsigned_object_id_type, osmium::Location>::instance();

        while (true) {
            const int c = getopt_long(argc, argv, "hi:I", long_options, nullptr);
            if (c == -1) {
                break;
            }

            switch (c) {
                case 'h':
                    print_help();
                    return exit_code_ok;
                case 'i':
                    location_index_type = optarg;
                    break;
                case 'I':
                    show_index_types();
                    return exit_code_ok;
                default:
                    return exit_code_cmdline_error;
            }
        }

        const int remaining_args = argc - optind;
        if (remaining_args != 1) {
            cerr << "Usage: " << argv[0] << " [OPTIONS] OSMFILE\n";
            return exit_code_cmdline_error;
        }
        auto location_index = map_factory.create_map(location_index_type);
        location_handler_type location_handler{*location_index};
        location_handler.ignore_errors(); // XXX
        const osmium::io::File input_file{argv[optind]};
        assembler_type::config_type assembler_config;
        assembler_config.create_empty_areas = true;

        mp_manager_type mp_manager{assembler_config};

        osmium::relations::read_relations(input_file, mp_manager);

        osmium::io::Reader reader2{input_file, entity_bits(location_index_type)};
        tag_counter counter;

        auto mp_manager_handler = mp_manager.handler([&counter](osmium::memory::Buffer&& buffer){
            for (const auto& area : buffer.select<osmium::Area>()) {
                if (area.num_rings().first == 0) {
                    if (area.tags().has_key("name")) {
                        ++counter.name;
                    }
                    if (area.tags().has_key("building")) {
                        ++counter.building;
                    } else if (area.tags().has_key("landuse")) {
                        ++counter.landuse;
                    } else if (area.tags().has_key("natural")) {
                        ++counter.natural;
                    } else if (area.tags().has_key("amenity")) {
                        ++counter.amenity;
                    } else if (area.tags().has_key("boundary")) {
                        ++counter.boundary;
                    } else if (area.tags().has_key("sport")) {
                        ++counter.sport;
                    } else if (area.tags().has_key("leisure")) {
                        ++counter.leisure;
                    } else if (area.tags().has_key("place")) {
                        ++counter.place;
                    } else if (area.tags().has_key("waterway")) {
                        ++counter.waterway;
                    } else {
                        ++counter.unknown;
                        for (const osmium::Tag& tag : area.tags()) {
                            cerr << area.id() << ' ' << tag.key() << ' ' << tag.value() << '\n';
                        }
                    }
                }
            }
        });
        if (location_index_type == "none") {
            osmium::apply(reader2, mp_manager_handler);
        } else {
            osmium::apply(reader2, location_handler, mp_manager_handler);
        }
        reader2.close();
        cout << "amenity:   " << counter.amenity  << '\n';
        cout << "boundary:  " << counter.boundary << '\n';
        cout << "building:  " << counter.building << '\n';
        cout << "place:     " << counter.place    << '\n';
        cout << "landuse:   " << counter.landuse  << '\n';
        cout << "natural:   " << counter.natural  << '\n';
        cout << "sport:     " << counter.sport    << '\n';
        cout << "unknown:   " << counter.unknown << '\n';
        cout << "leisure:   " << counter.leisure  << '\n';
        cout << "waterway:  " << counter.waterway << '\n';
        cout << "with name: " << counter.name << '\n';

    } catch (const exception& e) {
        cerr << e.what() << '\n';
        return exit_code_error;
    }

    return exit_code_ok;
}

