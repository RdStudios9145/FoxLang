#include <fstream>
#include <iostream>
#include <string>

#include <argparse.hpp>

auto main(int argc, char *argv[]) -> int {
  argparse::ArgumentParser program("fox", "0.0.1 epsilon");

  argparse::ArgumentParser compile_command("compile");
  compile_command.add_argument("-o", "--output").nargs(1);
  compile_command.add_argument("files").required().nargs(1);

  program.add_subparser(compile_command);

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }

  auto file_name = compile_command.get<std::string>("files");
  std::ifstream file(file_name);

  if (!file.is_open()) {
    std::cerr << "Could not open file `" << file_name << "`" << std::endl;
    std::exit(1);
  }

  std::string line;
  while (std::getline(file, line)) {
    std::cout << line << std::endl;
  }

  return 0;
}