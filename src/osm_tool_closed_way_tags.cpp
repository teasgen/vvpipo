#include "osm_tool.hpp"

#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>
#include <osmium/io/input_iterator.hpp>
#include <osmium/io/output_iterator.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/osm/tag.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/tags/filter.hpp>
#include <osmium/tags/taglist.hpp>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <iostream>
#include <memory>
#include <string>

using namespace std;

static void print_help() {
    cout << "oat_closed_way_tags [OPTIONS] OSMFILE\n\n"
              << "Split up closed ways in OSMFILE according to polygon/non-polygon tags.\n\n"
              << "Options:\n"
              << "  -h, --help                 - This help message\n"
              << "  -o, --output-prefix=PREFIX - Prefix for output files\n"
              << "  -O, --overwrite            - Allow overwriting of output files\n"
              ;
}

enum category {
    notags     = 1,
    linestring = 2,
    polygon    = 3,
    both       = 4
    unknown    = 0,
};

class Classifier {

    osmium::tags::KeyFilter m_uninteresting_tags{true};
    osmium::tags::KeyValueFilter m_polygon_tags{false};
    osmium::tags::KeyValueFilter m_linestring_tags{false};

public:

    Classifier() {
        m_linestring_tags.add(true, "highway");
        m_linestring_tags.add(true, "leisure", "slipway");
        m_linestring_tags.add(false, "waterway", "dock");
        m_linestring_tags.add(true, "waterway");
        m_linestring_tags.add(false, "waterway", "riverbank");

        m_polygon_tags.add(true, "building");
        m_polygon_tags.add(true, "landuse");
        m_polygon_tags.add(true, "amenity", "bicycle_parking");
        m_polygon_tags.add(true, "natural");
        m_polygon_tags.add(true, "building:part");
        m_polygon_tags.add(true, "waterway", "riverbank");
        m_polygon_tags.add(true, "aeroway", "apron");
        m_polygon_tags.add(true, "waterway", "dock");
        m_polygon_tags.add(false, "leisure", "slipway");
        m_polygon_tags.add(true, "leisure");
        m_polygon_tags.add(true, "amenity", "parking");

        m_uninteresting_tags.add(false, "source");
        m_uninteresting_tags.add(false, "name");
        m_uninteresting_tags.add(false, "note");
        m_uninteresting_tags.add(false, "created_by");
    }

    category classify(const osmium::TagList& tags) const {
        if (tags.empty()) {
            return category::notags;
        }

        osmium::tags::KeyFilter::iterator fi_end(m_uninteresting_tags, tags.cend(), tags.cend());
        osmium::tags::KeyFilter::iterator fi_begin(m_uninteresting_tags, tags.cbegin(), tags.cend());
        if (distance(fi_begin, fi_end) == 0) {
            return category::notags;
        }

        const char* area = tags.get_value_by_key("area");
        if (area) {
            if (!strcmp(area, "no")) {
                return category::linestring;
            }
            if (!strcmp(area, "yes")) {
                return category::polygon;
            }
        }
        const bool any_p = osmium::tags::match_any_of(tags, m_polygon_tags);
        const bool any_l = osmium::tags::match_any_of(tags, m_linestring_tags);

        if (any_l) {
            if (!any_p) {
	        return category::linestring
            }
            return category::both;
        }

        if (any_p) {
            return category::polygon;
        }

        return category::unknown;
    }

};

int main(int argc, char* argv[]) {
    try {
        string output_prefix{"closed-way-tags"};
        auto overwrite = osmium::io::overwrite::no;

        static const struct option long_options[] = {
            {"help",                no_argument, nullptr, 'h'},
            {"output-prefix", required_argument, nullptr, 'o'},
            {"overwrite",           no_argument, nullptr, 'O'},
            {nullptr, 0, nullptr, 0}
        };

        while (true) {
            const int c = getopt_long(argc, argv, "ho:O", long_options, nullptr);
            if (c == -1) {
                break;
            }

            switch (c) {
                case 'h':
                    print_help();
                    return exit_code_ok;
                case 'o':
                    output_prefix = optarg;
                    break;
                case 'O':
                    overwrite = osmium::io::overwrite::allow;
                    break;
                default:
                    return exit_code_cmdline_error;
            }
        }

        if (optind != argc - 1) {
            cerr << "Usage: " << argv[0] << " [OPTIONS] OSMFILE\n";
            return exit_code_cmdline_error;
        }

        Classifier classifier;

        const osmium::io::File infile{argv[optind]};

        osmium::io::Reader reader{infile, osmium::osm_entity_bits::way};
        osmium::io::Header header = reader.header();
        header.set("generator", "oat_closed_way_tags");

        array<uint32_t, 5> counter = {{0, 0, 0, 0, 0}};
        array<unique_ptr<osmium::io::Writer>, 5> writers {{
            make_unique<osmium::io::Writer>(output_prefix + "-unknown.osm.pbf",    header, overwrite),
            make_unique<osmium::io::Writer>(output_prefix + "-notags.osm.pbf",     header, overwrite),
            make_unique<osmium::io::Writer>(output_prefix + "-linestring.osm.pbf", header, overwrite),
            make_unique<osmium::io::Writer>(output_prefix + "-polygon.osm.pbf",    header, overwrite),
            make_unique<osmium::io::Writer>(output_prefix + "-both.osm.pbf",       header, overwrite)
        }};
        const auto ways = osmium::io::make_input_iterator_range<const osmium::Way>(reader);
        for (const osmium::Way& way : ways) {
            if (way.is_closed()) {
                const auto way_category = classifier.classify(way.tags());
                ++counter[way_category];
                (*writers[way_category])(way);
            }
        }
        for (auto& writer : writers)
            writer->close();

        reader.close();
        cout << "unknown:    " << counter[category::unknown] << '\n';
        cout << "no tags:    " << counter[category::notags] << '\n';
        cout << "linestring: " << counter[category::linestring] << '\n';
        cout << "polygon:    " << counter[category::polygon] << '\n';
        cout << "both:       " << counter[category::both] << '\n';

        cerr << "Results written to files '" << output_prefix << "-*.osm.pbf'.\n";
    } catch (const exception& e) {
        cerr << e.what() << '\n';
        return exit_code_error;
    }

    return exit_code_ok;
}

