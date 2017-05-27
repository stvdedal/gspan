#include "gspan_minimum_check.hpp"
#include <iostream>

int
main()
{
  using Ec = gspan::edgecodetree<char, char, boost::undirected_tag>;

  /*
   * example from
   * "gSpan: Graph-Based Substructure Pattern Mining" - Xifeng Yan, Jiawei Han
   * Table 1. d(gamma).
   *
   */

  Ec ec0(0, 1, 'X', 'X', 'a');
  Ec ec1(1, 2, 'X', 'Y', 'a', &ec0);
  Ec ec2(2, 0, 'Y', 'X', 'b', &ec1);
  Ec ec3(2, 3, 'Y', 'Z', 'b', &ec2);
  Ec ec4(3, 0, 'Z', 'X', 'c', &ec3);
  Ec ec5(2, 4, 'Y', 'Z', 'd', &ec4);

  print_dfsc(ec5, std::cout);
  if (is_minimum(ec5))
    std::cout << "is minumum" << std::endl;
  else
    std::cout << "is NOT minumum" << std::endl;
}
