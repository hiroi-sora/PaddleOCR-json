#include <iostream>

#include "args.hpp"


int main(int argc, char **argv)
{
  if (argc <= 1)
  {
    std::cout << "Try ./example -help" << std::endl;
    return 0;
  }
  GFLAGS_NAMESPACE::SetUsageMessage("Example compilation for demo.");
  GFLAGS_NAMESPACE::ParseCommandLineFlags(&argc, &argv, true);
  print_message();
  return 0;
}
