#include "args.hpp"

#include <iostream>

DEFINE_string(message, "", "The message to print");
DEFINE_double(number, 0.3, "The number");

void print_message()
{
  std::cout << FLAGS_message << std::endl;
}
