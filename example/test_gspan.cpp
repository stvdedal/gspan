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
#include <string>
#include <sstream>
#include <algorithm>
#include <list>
#include <vector>
#include <cstdlib>

using namespace boost;

using VP = property<vertex_name_t, std::size_t>;
using EP = property<edge_index_t, std::size_t, property<edge_name_t, std::size_t> >;
using InputGraph = adjacency_list<vecS, vecS, undirectedS, VP, EP, std::size_t>;
using InputGraphVertex = graph_traits<InputGraph>::vertex_descriptor;
using InputGraphEdge = graph_traits<InputGraph>::edge_descriptor;
using GspanTraits = gspan_traits<InputGraph, vertex_name_t, edge_name_t>;

std::vector<std::string> v_values;
std::vector<std::string> e_values;

// output options
enum OutputMappings
{
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
result(const GspanTraits::MG& mg, const GspanTraits::SG& sg, int support)
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

std::string&
remove_comment(std::string& s)
{
  s.erase(std::find(s.begin(), s.end(), '#'), s.end());
  return s;
}
const char* whitespaces = " \t\n\r\f\v";
std::string&
remove_whitespaces_left(std::string& s)
{
  std::size_t p = s.find_first_not_of(whitespaces);
  if (p != std::string::npos)
    s.erase(s.begin(), s.begin() + p);
  return s;
}
std::string&
remove_whitespaces_right(std::string& s)
{
  std::size_t p = s.find_last_not_of(whitespaces);
  if (p != std::string::npos)
    s.erase(p + 1);
  return s;
}

void
print_usage(std::ostream& s)
{
  s << "Usage: gspan [ <minsupport> ] [OPTION]" << std::endl
    << "Graph-based substructure pattern mining." << std::endl
    << "Read input data from standard input, write result to standard output."
    << std::endl << "OPTION is" << std::endl
    << "  <minsupport>  minimal pattern occurence, default 1" << std::endl
    << "  -om           output pattern to input graph mapping options:"
    << std::endl
    << "                  none, autgrp, all, default. default is none"
    << std::endl << std::endl;
}

int
main(int argc, char** argv)
{
  unsigned int min_support = 1;

  if (argc > 1) {
    std::stringstream ss(argv[1]);
    if (!(ss >> min_support))
      min_support = 1;
  }

  for (int i = 1; i < argc; ++i) {
    std::string arg(argv[i]);
    if (arg == "-h" || arg == "--help") {
      print_usage(std::cout);
      return 0;
    } else if (arg == "-om") {
      if (i + 1 < argc) {
        std::string param(argv[i + 1]);
        if (param == "autgrp") {
          output_mappings = OUTPUT_MAPPING_ONE_AUTOMORPH;
        }
        if (param == "all") {
          output_mappings = OUTPUT_MAPPING_ALL;
        }
        continue;
      } else {
        print_usage(std::cerr);
        return 1;
      }
    }
  }

  std::list<InputGraph> input_graphs;

  /// input parsing and fill input_graphs
  int line_no = 0;
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
    case 't':
    {
      int graph_id = 0;
      ss >> graph_id;
      if (!ss) {
        std::cerr << "invalid or missed <graph_id>, at line " << line_no
                  << std::endl;
        return 1;
      }
      input_graphs.push_back(InputGraph());
      input_graphs.back()[graph_bundle] = graph_id;
    }
      break;
    case 'v':
    {
      if (input_graphs.empty()) {
        std::cerr << "invalid format: 't' tag missed" << std::endl;
        return 1;
      }
      unsigned int vertex_id = 0;
      if (!(ss >> vertex_id)) {
        std::cerr << "invalid or missed <vertex_id>, at line " << line_no
                  << std::endl;
        return 1;
      }
      std::string value;
      getline(ss, value);
      remove_whitespaces_left(value);

      auto valit = std::find(v_values.begin(), v_values.end(), value);
      if (valit == v_values.end()) {
        valit = v_values.insert(v_values.end(), value);
      }
      InputGraphVertex u = add_vertex(input_graphs.back());
      put(get(vertex_name, input_graphs.back()), u, valit - v_values.begin());
    }
      break;
    case 'e':
    {
      if (input_graphs.empty()) {
        std::cerr << "invalid format: 't' tag missed" << std::endl;
        return 1;
      }
      InputGraph& g = input_graphs.back();
      unsigned int edge_id = 0;
      if (!(ss >> edge_id)) {
        std::cerr << "invalid or missed <edge_id>, at line " << line_no
                  << std::endl;
        return 1;
      }
      unsigned int vertex_id_1, vertex_id_2;
      if (!(ss >> vertex_id_1) || vertex_id_1 >= num_vertices(g)) {
        std::cerr << "invalid or missed <vertex_id_1>, at line " << line_no
                  << std::endl;
        return 1;
      }
      if (!(ss >> vertex_id_2) || vertex_id_2 >= num_vertices(g)) {
        std::cerr << "invalid or missed <vertex_id_2>, at line " << line_no
                  << " vertex_id_2=" << vertex_id_2 << std::endl;
        return 1;
      }
      std::string value;
      getline(ss, value);
      remove_whitespaces_left(value);

      auto valit = std::find(e_values.begin(), e_values.end(), value);
      if (valit == e_values.end()) {
        valit = e_values.insert(e_values.end(), value);
      }

      InputGraphEdge e = add_edge(vertex_id_1, vertex_id_2, g).first;
      put(get(edge_name, g), e, valit - e_values.begin());
      put(get(edge_index, g), e, num_edges(g) - 1);
    }
      break;
    default:
      std::cerr << "invalid or missed <tag>, at line " << line_no << std::endl;
      return 1;
    }
  }

  if (input_graphs.size() == 1)
    gspan_one_graph(input_graphs.back(),
                    min_support,
                    result,
                    vertex_name_t(),
                    edge_name_t());
  else
    gspan_many_graphs(input_graphs.begin(),
                      input_graphs.end(),
                      min_support,
                      result,
                      vertex_name_t(),
                      edge_name_t());
}
