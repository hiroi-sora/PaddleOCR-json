#include "args.hpp"

#include <iostream>

DEFINE_string(message, "", "The message to print");

void print_message()
{
  std::cout << FLAGS_message << std::endl;
}
