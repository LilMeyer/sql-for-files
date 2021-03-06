#include <cctype>
#include <fstream>
#include <cassert>
#include <fstream>
#include <vector>

#include "driver.hpp"
#include "util/vec.hpp"

sqlforfiles::Driver::~Driver() {
  delete(scanner);
  scanner = nullptr;
  delete(parser);
  parser = nullptr;
}

void sqlforfiles::Driver::parse(const char * const filename) {
  assert(filename != nullptr);
  std::ifstream in_file(filename);

  if (!in_file.good()) {
    std::cout << "can't open file filename" << std::endl;
    exit(EXIT_FAILURE);
  }

  set_mode_interactive(false);

  parse_helper(in_file);
  return;
}

void sqlforfiles::Driver::parse(std::istream &stream) {
  if (!stream.good() && stream.eof()) {
    return;
  }

  set_mode_interactive(true);

  parse_helper(stream);
  return;
}

void sqlforfiles::Driver::parse_helper(std::istream &stream) {
  if (this->mode_interactive) {
    std::cout << "=>";
  }
  delete(scanner);
  try {
    scanner = new sqlforfiles::Scanner(&stream);
  } catch (std::bad_alloc &ba) {
    std::cerr << "Failed to allocate scanner: (" <<
      ba.what() << "), exiting!!\n";
    exit(EXIT_FAILURE);
  }
  delete(parser);

  try {
    parser = new sqlforfiles::Parser(
      (*scanner), /* scanner */
      (*this)     /* driver */
    );
  } catch (std::bad_alloc &ba) {
    std::cerr << "Failed to allocate parser: (" <<
      ba.what() << "), exiting!!\n";
    exit(EXIT_FAILURE);
  }
  const int accept(0);

  if (parser->parse() != accept) {
    std::cerr << "Parse failed!!\n";
  }
  // else {
  //   std::cout << "parse ok" << std::endl;
  // }

  stream.clear();

  if (this->mode_interactive) {
    parse_helper(stream);
  }

  return;
}

void sqlforfiles::Driver::set_mode_interactive(bool is_interactive) {
  this->mode_interactive = is_interactive;
}

void sqlforfiles::Driver::set_limit(const int &limit) {
  this->limit = limit;
}

void sqlforfiles::Driver::add_filename(const std::string &filename) {
  this->filename = filename;
}

void sqlforfiles::Driver::add_field_selection(const int &i) {
  select.push_back(i);
}

void sqlforfiles::Driver::set_delimiter(const std::string &delimiter) {
  this->delimiter = delimiter[1];
}


template <class T>
std::ostream& operator<< (std::ostream& out, const std::vector<T>& v) {
  out << "[";
  size_t last = v.size() - 1;
  for(size_t i = 0; i < v.size(); ++i) {
    out << v[i];
    if (i != last) {
      out << ", ";
    }
  }
  out << "]";
  return out;
}


std::ostream& sqlforfiles::Driver::process_query(std::ostream &stream) {
  int lineNb = 0;
  std::string line;
  std::ifstream file(filename);
  int prevOcc, i, l;
  std::vector<std::string> buff, result;
  std::vector<std::string>::iterator it;


  if (file.is_open()) {
    while (getline(file, line)) {
      lineNb++;
      if (this->limit && lineNb > this->limit) {
        break;
      }

      prevOcc = -1;
      buff.clear();
      for (i = 0, l = line.size(); i < l; i++) {
        if (line[i] == this->delimiter) {
          buff.push_back(line.substr(prevOcc + 1, i - prevOcc - 1));
          prevOcc = i;
        }
      }

      buff.push_back(line.substr(prevOcc + 1, l - prevOcc));
      result = Vec::selector(buff, this->select);

      Vec::output(std::cout, result, ';');
    }
    file.close();
  } else {
    stream << "Unable to open file " << filename << std::endl;
  }

  if (this->mode_interactive) {
    stream << "=>";
  }

  return stream;
}
