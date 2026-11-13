#include "mini/Version.h"

#include <iostream>
#include <string_view>

namespace {

void printUsage(std::ostream& out) {
  out << "usage: mini-opt --version\n"
         "       mini-opt --help\n";
}

}  // namespace

int main(int argc, char* argv[]) {
  if (argc == 2) {
    const std::string_view argument = argv[1];
    if (argument == "--version") {
      std::cout << "mini-opt " << mini::version() << '\n';
      return 0;
    }
    if (argument == "--help") {
      printUsage(std::cout);
      return 0;
    }
  }

  printUsage(std::cerr);
  return 2;
}
