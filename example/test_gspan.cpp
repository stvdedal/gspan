/**
 * \file
 * \author stvdedal@gmail.com
 *
 * \brief
 * Program for testing gspan algorithm
 */

#include "gspan.hpp"

#include <boost/graph/adjacency_list.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <functional>

#include <cctype>
#include <cstdlib>

using namespace boost;

void
print_usage(std::ostream& s)
{
    // line width 80
    // --------------------------------------------------------------------------------
    s <<
      "Usage: gspan [options]\n"
      "Graph-based substructure pattern mining.\n"
      "Depending on the graph count in input, there are two modes:\n"
      "  1. input contains one graph. Mined patterns belong to this one;\n"
      "       in this case only --mincount=NUM option is used\n"
      "  2. input contains many graphs. Mined patterns belong to some graph in input;\n"
      "       in this case --minsupp=NUM option is used, as more useful.\n"
      "Options:\n"
      "  -i, --input FILE        file to read, default stdin\n"
      "  -o, --output FILE       file to write, default stdout\n"
      "  -c, --mincount NUM      minimal count, integer value, default 1\n"
      "  -s, --minsupp NUM       minimal support, 0..1\n"
      "  -l, --legacy            use tgf format for input and output (slower!)\n"
      "  -e, --embeddings [opts] none, autgrp, all. default is none\n"
      "  -h, --help              this help"
      << std::endl;
}

void error_usage()
{
    print_usage(std::cerr);
    exit(1);
}

std::ifstream input_fstream;
std::ofstream output_fstream;
std::istream* input_stream = &std::cin;
std::ostream* output_stream = &std::cout;
bool no_output = false;
bool use_legacy = false;
enum OutputMappings {
    OUTPUT_MAPPING_NONE,
    OUTPUT_MAPPING_ONE_AUTOMORPH,
    OUTPUT_MAPPING_ALL
} output_mappings = OUTPUT_MAPPING_NONE;

static std::size_t pattern_no = 0;

/**
 * Vertex and edge properties are used by algorithm.
 * To optimize comparison, they are integers (not strings)
 *
 * vertex values are addressed by boost::vertex_name
 * edge values   are addressed by boost::edge_name
 */
std::vector<std::string> v_values;
std::vector<std::string> e_values;

std::size_t
map_string_to_integer(std::vector<std::string>& values,
                      const std::string& value)
{
    auto valit = std::find(values.begin(), values.end(), value);
    if (valit == values.end()) {
        valit = values.insert(values.end(), value);
    }
    return valit - values.begin();
}

using VP = property<vertex_name_t, std::size_t>;
using EP =
    property<edge_index_t, std::size_t, property<edge_name_t, std::size_t> >;

using InputGraph = adjacency_list<vecS, vecS, undirectedS, VP, EP, std::size_t>;
using InputGraphVertex = graph_traits<InputGraph>::vertex_descriptor;
using InputGraphEdge = graph_traits<InputGraph>::edge_descriptor;
using GspanTraits = gspan_traits<InputGraph, vertex_name_t, edge_name_t>;

template <typename MG, typename SBG>
void
print_mapping(const MG& mg,
              const SBG& s,
              std::size_t map_no,
              std::size_t autmorph_no)
{
    std::ostream& os = *output_stream;

    os << std::endl;
    os << "m " << map_no << " # automorh " << autmorph_no << std::endl;
    const InputGraph& ig = *s.input_graph();
    for (auto v_mg : vertices(mg)) {
        auto v_ig = get_v_ig(s, v_mg);
        os << "v " << v_index(mg, v_mg) << " ";
        os << ig[graph_bundle] << " ";
        os << get(get(vertex_index, ig), v_ig) << std::endl;
    }
    for (auto e_mg : edges(mg)) {
        auto e_ig = get_e_ig(s, e_mg);
        os << "e " << e_index(mg, e_mg) << " ";
        os << ig[graph_bundle] << " ";
        os << get(get(edge_index, ig), e_ig) << std::endl;
    }
}

void
write_egf(const GspanTraits::MG& mg, const GspanTraits::SG& sg, int support)
{
    ++pattern_no;

    if (no_output)
        return;

    std::ostream& os = *output_stream;

    os << std::endl;
    os << "p " << pattern_no << " # occurence " << support << std::endl;
    for (auto v : vertices(mg))
        os << "v " << v_index(mg, v) << " " << v_values[v_bundle(mg, v)]
           << std::endl;
    for (auto e : edges(mg))
        os << "e " << e_index(mg, e) << " " << source_index(mg, e) << " "
           << target_index(mg, e) << " " << e_values[e_bundle(mg, e)] << std::endl;

    if (output_mappings != OUTPUT_MAPPING_NONE) {
        std::size_t map_no = 0;
        for (const auto& g_sbgs : sg) {
            for (const auto& grp : g_sbgs.second.aut_list) {
                std::size_t autmorph_no = 0;
                for (const auto& s : grp) {
                    print_mapping(mg, *s, ++map_no, ++autmorph_no);
                    if (output_mappings == OUTPUT_MAPPING_ONE_AUTOMORPH)
                        break;
                }
            }
        }
    }
}

void
write_tgf(const GspanTraits::MG& mg, const GspanTraits::SG& sg, int support)
{
    ++pattern_no;

    if (no_output)
        return;
    std::ostream& os = *output_stream;

    using MGE = GspanTraits::MG::edge_descriptor;
    std::vector<MGE> mg_edges; // to reverse (for matching with gbolt)

    mg_edges.reserve(num_edges(mg));
    for (auto e : edges(mg))
        mg_edges.push_back(e);

    os << "t # " << pattern_no - 1 << " * " << support << std::endl;
    for (auto v : vertices(mg))
        os << "v " << v_index(mg, v) << " " << v_bundle(mg, v) << std::endl;

    using RevIt = std::vector<MGE>::const_reverse_iterator;
    for (RevIt ei = mg_edges.rbegin(); ei != mg_edges.rend(); ++ei) {
        MGE e = *ei;
        os << "e " << source_index(mg, e) << " " << target_index(mg, e)
           << " " << e_bundle(mg, e) << std::endl;
    }

    std::set<std::size_t> graph_ids;
    for (const auto& g_sbgs : sg) {
        const InputGraph& ig = *g_sbgs.first;
        graph_ids.insert(ig[graph_bundle]);
    }

    os << "x: ";
    for (std::size_t graph_id : graph_ids) {
        os << graph_id << " ";
    }

    os << std::endl << std::endl;
}

inline
std::string&
remove_comment(std::string& s)
{
    s.erase(std::find(s.begin(), s.end(), '#'), s.end());
    return s;
}

inline
std::string&
remove_whitespaces_left(std::string& s)
{
    using namespace std;
    s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun(&::isblank))));
    return s;
}

inline
std::string&
remove_whitespaces_right(std::string& s)
{
    using namespace std;
    auto rit = find_if(s.rbegin(), s.rend(), not1(ptr_fun(&::isblank)));
    s.erase(rit.base(), s.end());
    return s;
}

bool read_egf(std::list<InputGraph>& container, std::istream& is)
{
    std::map<std::size_t, InputGraphVertex> vmap;
    std::size_t line_no = 0;
    std::string line;

    while (getline(is, line)) {
        ++line_no;
        remove_comment(line);
        remove_whitespaces_left(line);
        remove_whitespaces_right(line);
        if (line.empty())
            continue;
        char tag = 0;
        std::stringstream ss(line);
        ss >> tag;
        switch (tag) {
        case 't': {
            vmap.clear();
            container.push_back(InputGraph());
            std::size_t graph_id = 0;
            ss >> graph_id;
            if (!ss) {
                std::cerr << "invalid or missed <graph_id>, at line " << line_no
                          << std::endl;
                return false;
            }
            container.back()[graph_bundle] = graph_id;
        }
        break;
        case 'v': {
            if (container.empty()) {
                std::cerr << "invalid format: 't' tag missed" << std::endl;
                return false;
            }
            InputGraph& g = container.back();
            InputGraphVertex v = add_vertex(g);
            std::size_t vertex_id = 0;
            ss >> vertex_id;
            if (!ss) {
                std::cerr << "invalid or missed <vertex_id>, at line " << line_no
                          << std::endl;
                return false;
            }

            std::string value;
            getline(ss, value);
            remove_whitespaces_left(value);
            put(get(vertex_name, g), v, map_string_to_integer(v_values, value));
            vmap[vertex_id] = v;
        }
        break;
        case 'e': {
            if (container.empty()) {
                std::cerr << "invalid format: 't' tag missed" << std::endl;
                return false;
            }
            InputGraph& g = container.back();
            std::size_t edge_id = 0;
            if (!(ss >> edge_id)) {
                std::cerr << "invalid or missed <edge_id>, at line " << line_no
                          << std::endl;
                return false;
            }
            std::size_t src_id;
            if (!(ss >> src_id) || vmap.find(src_id) == vmap.end()) {
                std::cerr << "invalid or missed <vertex_id_1>, at line " << line_no
                          << std::endl;
                return false;
            }
            std::size_t dst_id;
            if (!(ss >> dst_id) || vmap.find(dst_id) == vmap.end()) {
                std::cerr << "invalid or missed <vertex_id_2>, at line " << line_no
                          << std::endl;
                return false;
            }

            InputGraphVertex u = vmap[src_id];
            InputGraphVertex v = vmap[dst_id];
            InputGraphEdge e = add_edge(u, v, g).first;
            put(get(edge_index, g), e, edge_id);

            std::string value;
            getline(ss, value);
            remove_whitespaces_left(value);
            put(get(edge_name, g), e, map_string_to_integer(e_values, value));
        }
        break;
        default:
            std::cerr << "invalid or missed <tag>, at line " << line_no << std::endl;
            return false;
        }
    }

    return true;
}

bool read_tgf(std::list<InputGraph>& container, std::istream& is)
{
    std::map<std::size_t, InputGraphVertex> vmap;
    std::size_t line_no = 0;
    std::string line;

    while (getline(is, line)) {
        ++line_no;
        if (line.empty())
            continue;
        char tag = 0;
        std::stringstream ss(line);
        ss >> tag;
        switch (tag) {
        case 't': {
            vmap.clear();
            container.push_back(InputGraph());
            char nsign = 0;
            std::size_t graph_id = 0;
            ss >> nsign >> graph_id;
            if (!ss || nsign != '#') {
                std::cerr << "invalid or missed <graph_id>, at line " << line_no
                          << std::endl;
                return false;
            }
            container.back()[graph_bundle] = graph_id;
        }
        break;
        case 'v': {
            if (container.empty()) {
                std::cerr << "invalid format: 't' tag missed" << std::endl;
                return false;
            }
            InputGraph& g = container.back();
            InputGraphVertex v = add_vertex(g);
            std::size_t vertex_id = 0;
            ss >> vertex_id;
            if (!ss || vertex_id >= num_vertices(g)) {
                std::cerr << "invalid or missed <vertex_id>, at line " << line_no
                          << std::endl;
                return false;
            }

            std::size_t ival;
            if (!(ss >> ival)) {
                std::cerr << "invalid or missed vertex value (integer), at line " << line_no
                          << std::endl;
                return false;
            }
            put(get(vertex_name, g), v, ival);
            vmap[vertex_id] = v;
        }
        break;
        case 'e': {
            if (container.empty()) {
                std::cerr << "invalid format: 't' tag missed" << std::endl;
                return false;
            }
            InputGraph& g = container.back();
            std::size_t src_id;
            if (!(ss >> src_id) || vmap.find(src_id) == vmap.end()) {
                std::cerr << "invalid or missed <vertex_id_1>, at line " << line_no
                          << std::endl;
                return false;
            }
            std::size_t dst_id;
            if (!(ss >> dst_id) || vmap.find(dst_id) == vmap.end()) {
                std::cerr << "invalid or missed <vertex_id_2>, at line " << line_no
                          << std::endl;
                return false;
            }

            InputGraphVertex u = vmap[src_id];
            InputGraphVertex v = vmap[dst_id];
            InputGraphEdge e = add_edge(u, v, g).first;
            put(get(edge_index, g), e, num_edges(g) - 1);

            std::size_t ival;
            if (!(ss >> ival)) {
                std::cerr << "invalid or missed edge value (integer), at line " << line_no
                          << std::endl;
                return false;
            }
            put(get(edge_name, g), e, ival);
        }
        break;
        default:
            std::cerr << "invalid or missed <tag>, at line " << line_no << std::endl;
            return false;
        }
    }

    return true;
}

struct input_statistics {
    std::size_t graph_count;
    struct {
        std::size_t avg;
        std::size_t min;
        std::size_t max;
    } v, e;
};

void calculate_statistics(const std::list<InputGraph>& container,
                          input_statistics* stat)
{
    stat->graph_count = 0;
    stat->v.avg = 0;
    stat->v.min = 0;
    stat->v.max = 0;
    stat->e.avg = 0;
    stat->e.min = 0;
    stat->e.max = 0;
    if (container.empty()) {
        return;
    }
    stat->v.max = stat->v.min = num_vertices(container.front());
    stat->e.max = stat->e.min = num_edges(container.front());
    for (const InputGraph& g : container) {
        ++stat->graph_count;
        std::size_t vn = num_vertices(g);
        std::size_t en = num_edges(g);
        stat->v.avg += vn;
        stat->e.avg += en;
        if (stat->v.min > vn) {
            stat->v.min = vn;
        }
        if (stat->v.max < vn) {
            stat->v.max = vn;
        }
        if (stat->e.min > en) {
            stat->e.min = en;
        }
        if (stat->e.max < en) {
            stat->e.max = en;
        }
    }
    stat->v.avg /= stat->graph_count;
    stat->e.avg /= stat->graph_count;
}

int
main(int argc, char** argv)
{
    std::size_t mincount = 0;
    bool minsupp_exist = true;
    double minsupp = 1.0;

    for (int i = 1; i < argc; ++i) {
        std::string opt(argv[i]);
        if (opt == "--help" || opt == "-h") {
            print_usage(std::cout);
            return 0;
        }
        else if (opt == "--input" || opt == "-i") {
            if (++i >= argc || input_fstream.is_open())
                error_usage();
            input_fstream.open(argv[i]);
            input_stream = &input_fstream;
            continue;
        }
        else if (opt == "--output" || opt == "-o") {
            if (++i >= argc || output_fstream.is_open())
                error_usage();
            std::string file(argv[i]);
            if (file != "/dev/null") {
                output_fstream.open(file);
                output_stream = &output_fstream;
            }
            else {
                no_output = true;
            }
            continue;
        }
        else if (opt == "--mincount" || opt == "-c") {
            if (++i >= argc)
                error_usage();
            if (! (std::stringstream(argv[i]) >> mincount)) {
                error_usage();
            }
            continue;
        }
        else if (opt == "--minsupp" || opt == "-s") {
            if (++i >= argc)
                error_usage();
            if (! (std::stringstream(argv[i]) >> minsupp)) {
                error_usage();
            }
            minsupp_exist = true;
            continue;
        }
        else if (opt == "--legacy" || opt == "-l") {
            use_legacy = true;
        }
        else if (opt == "--embeddings" || opt == "-e") {
            if (++i >= argc)
                error_usage();
            std::string param(argv[i]);
            if (param == "all")
                output_mappings = OUTPUT_MAPPING_ALL;
            else if (param == "autgrp")
                output_mappings = OUTPUT_MAPPING_ONE_AUTOMORPH;
            else if (param == "none")
                output_mappings = OUTPUT_MAPPING_NONE;
            else
                error_usage();
            continue;
        }
        else {
            error_usage();
        }
    }

    std::list<InputGraph> input_graphs;

    if (!(use_legacy ? read_tgf : read_egf)(input_graphs, *input_stream))
        return 1;

    input_statistics stat;
    calculate_statistics(input_graphs, &stat);

    if (minsupp_exist) {
        mincount = stat.graph_count * minsupp;
    }

    std::cerr << std::endl;
    std::cerr << "# input data statistics:\n"
              << "# graph count          = " << stat.graph_count << std::endl
              << "# vertices avg,min,max = "
              << stat.v.avg << ", " << stat.v.min << ", " << stat.v.max << std::endl
              << "# edges avg,min,max    = "
              << stat.e.avg << ", " << stat.e.min << ", " << stat.e.max << std::endl
              << "# min_count            = " << mincount << std::endl << std::endl;

    if (input_graphs.size() == 1)
        gspan_one_graph(input_graphs.back(),
                        mincount,
                        use_legacy ? write_tgf : write_egf,
                        vertex_name,
                        edge_name);
    else
        gspan_many_graphs(input_graphs.begin(),
                          input_graphs.end(),
                          mincount,
                          use_legacy ? write_tgf : write_egf,
                          vertex_name,
                          edge_name);

    std::cerr << std::endl;
    std::cerr << "# mined " << pattern_no << " patterns" << std::endl;
}
