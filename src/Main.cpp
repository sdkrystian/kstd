#include <iostream>
#include "kvector.h"

struct bar
{
  bar() { std::cout << "default\n"; }
  bar(const bar&) { std::cout << "copy\n"; }
  bar(bar&& b) noexcept { std::cout << "move\n"; }
  ~bar() { std::cout << "dtor\n"; }
};

int main()
{
  kstd::vector<bar> a;
  a.emplace_back();
  a.reserve(20);
  a.pop_back();
  std::cin.ignore();
  return 0;
}