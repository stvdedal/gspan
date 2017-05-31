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
#include <cctype>
#include <list>
#include <vector>
#include <map>
#include <algorithm>
#include <functional>

using namespace boost;

void
print_usage(std::ostream& s)
{
    s <<
      "Usage: gspan [ <minsupport> ] [OPTION]\n"
      "Graph-based substructure pattern mining.\n"
      "Read input data from standard input, write result to standard output.\n"
      "OPTION is\n"
      "  -ofile           file to output\n"
      "  <-minoccurence>  minimal pattern occurence, in graph count, default 1\n"
      "  <-minsupport>    minimal pattern occurence, 0..1\n"
      "  -tgf             use tgf format for input and output\n"
      "  -om              output pattern to input graph mapping options:\n"
      "                     none, autgrp, all, default. default is none\n"
      << std::endl;
}

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


// options

bool use_egf = true;

enum OutputMappings {
    OUTPUT_MAPPING_NONE, OUTPUT_MAPPING_ONE_AUTOMORPH, OUTPUT_MAPPING_ALL
} output_mappings = OUTPUT_MAPPING_NONE;

template <typename MG, typename SBG>
void
print_mapping(const MG& mg,
              const SBG& s,
              std::size_t map_no,
              std::size_t autmorph_no)
{
    std::cout << std::endl;
    std::cout << "m " << map_no << " # automorh " << autmorph_no << std::endl;
    const InputGraph& ig = *s.input_graph();
    for (auto v_mg : vertices(mg)) {
        auto v_ig = get_v_ig(s, v_mg);
        std::cout << "v " << v_index(mg, v_mg) << " ";
        std::cout << ig[graph_bundle] << " ";
        std::cout << get(get(vertex_index, ig), v_ig) << std::endl;
    }
    for (auto e_mg : edges(mg)) {
        auto e_ig = get_e_ig(s, e_mg);
        std::cout << "e " << e_index(mg, e_mg) << " ";
        std::cout << ig[graph_bundle] << " ";
        std::cout << get(get(edge_index, ig), e_ig) << std::endl;
    }
}

void
write_egf(const GspanTraits::MG& mg, const GspanTraits::SG& sg, int support)
{
    static std::size_t pattern_no = 0;
    ++pattern_no;

    std::cout << std::endl;
    std::cout << "p " << pattern_no << " # occurence " << support << std::endl;
    for (auto v : vertices(mg))
        std::cout << "v " << v_index(mg, v) << " " << v_values[v_bundle(mg, v)]
                  << std::endl;
    for (auto e : edges(mg))
        std::cout << "e " << e_index(mg, e) << " " << source_index(mg, e) << " "
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
    static std::size_t pattern_no = 0;

    std::cout << "t # " << pattern_no << " * " << support << std::endl;
    for (auto v : vertices(mg))
        std::cout << "v " << v_index(mg, v) << " " << v_bundle(mg, v) << std::endl;
    for (auto e : edges(mg))
        std::cout << "e " << source_index(mg, e) << " " << target_index(mg, e)
                  << " " << e_bundle(mg, e) << std::endl;
    std::cout << "x: ";
    for (const auto& g_sbgs : sg) {
        const InputGraph& ig = *g_sbgs.first;
        std::cout << ig[graph_bundle] << " ";
    }
    std::cout << std::endl << std::endl;
    ++pattern_no;
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

    while (getline(std::cin, line)) {
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

    while (getline(std::cin, line)) {
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

    std::size_t g_tot, v_tot, e_tot, v_min, v_max, e_min, e_max;
    g_tot = v_tot = e_tot = 0;

    v_min = 0;
    v_max = 0;
    e_min = 0;
    e_max = 0;

    v_max = v_min = num_vertices(container.front());
    e_max = e_min = num_edges(container.front());

    for (const InputGraph& g : container) {
        ++g_tot;
        std::size_t vn = num_vertices(g);
        std::size_t en = num_edges(g);
        v_tot += vn;
        e_tot += en;

        if (v_min > vn) {
            v_min = vn;
        }
        if (e_min > en) {
            e_min = en;
        }
        if (v_max < vn) {
            v_max = vn;
        }
        if (e_max < en) {
            e_max = en;
        }
    }

    stat->graph_count = g_tot;
    stat->v.avg = v_tot / g_tot;
    stat->v.min = v_min;
    stat->v.max = v_max;
    stat->e.avg = e_tot / g_tot;
    stat->e.min = e_min;
    stat->e.max = e_max;
}

int
main(int argc, char** argv)
{
    std::list<InputGraph> input_graphs;

    std::size_t min_occurence = 1;
    bool use_support = false;
    double min_support = 0;

    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg == "-h" || arg == "--help") {
            print_usage(std::cout);
            return 0;
        }
        else if (arg == "-minoccurence") {
            if (i + 1 < argc) {
                std::string param(argv[i + 1]);
                std::stringstream ss(param);
                ss >> min_occurence;
                if (!ss) {
                    print_usage(std::cerr);
                    return 1;
                }
            }
            else {
                print_usage(std::cerr);
                return 1;
            }
        }
        else if (arg == "-minsupport") {
            if (i + 1 < argc) {
                use_support = true;
                std::string param(argv[i + 1]);
                std::stringstream ss(param);
                ss >> min_support;
                if (!ss) {
                    print_usage(std::cerr);
                    return 1;
                }
            }
            else {
                print_usage(std::cerr);
                return 1;
            }
        }
        else if (arg == "-om") {
            if (i + 1 < argc) {
                std::string param(argv[i + 1]);
                if (param == "autgrp") {
                    output_mappings = OUTPUT_MAPPING_ONE_AUTOMORPH;
                }
                if (param == "all") {
                    output_mappings = OUTPUT_MAPPING_ALL;
                }
                continue;
            }
            else {
                print_usage(std::cerr);
                return 1;
            }
        }
        else if (arg == "-tgf") {
            use_egf = false;
        }
    }

    if (!(use_egf ? read_egf : read_tgf)(input_graphs, std::cin))
        return 1;

    input_statistics stat;
    calculate_statistics(input_graphs, &stat);

    if (use_support) {
        min_occurence = stat.graph_count * min_support;
    }

#if 0
    std::cout << "# input stat:\n"
              << "# graph count          = " << stat.graph_count << std::endl
              << "# vertices avg,min,max = "
              << stat.v.avg << ", " << stat.v.min << ", " << stat.v.max << std::endl
              << "# edges avg,min,max    = "
              << stat.e.avg << ", " << stat.e.min << ", " << stat.e.max << std::endl
              << "# min_occurence        = " << min_occurence << std::endl << std::endl;
#endif

    if (input_graphs.size() == 1)
        gspan_one_graph(input_graphs.back(),
                        min_occurence,
                        use_egf ? write_egf : write_tgf,
                        vertex_name,
                        edge_name);
    else
        gspan_many_graphs(input_graphs.begin(),
                          input_graphs.end(),
                          min_occurence,
                          use_egf ? write_egf : write_tgf,
                          vertex_name,
                          edge_name);
}
