#include <iostream>
#include "kvector.h"
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <cstdint>

struct bar
{
  bar(const std::string& t = "") : tag(t) { std::cout << "default: " << tag <<"\n"; }
  bar(const bar& other) : tag(other.tag) { std::cout << "copy: " << tag << "\n"; }
  bar(bar&& other) noexcept : tag(other.tag) { std::cout << "move: " << tag << "\n"; }
  bar& operator=(const bar& other) { tag = other.tag; std::cout << "copy assign: " << tag << "\n"; return *this; }
  bar& operator=(bar&& other) { tag = other.tag; std::cout << "move assign: " << tag << "\n"; return *this; }
  ~bar() { std::cout << "dtor: " << tag << "\n"; }

  std::string tag;
};

template<template<typename, typename> typename T, typename U, typename V = std::allocator<U>>
std::uint64_t vector_benchmark(std::uint64_t n)
{
  T<U, V> vec;
  vec.reserve(n);
  std::chrono::time_point start = std::chrono::high_resolution_clock::now();
  for (std::uint64_t i = 0; i < n / 2; ++i)
    vec.push_back(U("hello"));
  vec.insert(vec.begin(), n / 2, U());
  std::chrono::time_point end = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

int main()
{
  constexpr uint64_t cycles = 2000;
  std::random_device random;
  std::uniform_int_distribution dist(1, 2000);
  std::uint64_t counter_std = 0;
  std::uint64_t counter_kstd = 0;
  std::cout << "Running benchmark...\n";
  for (int i = 0; i < cycles; ++i)
  {
    counter_kstd += vector_benchmark<kstd::vector, std::string>(dist(random));
    counter_std += vector_benchmark<std::vector, std::string>(dist(random));
  }
  std::cout << "std::vector: " << (counter_std / cycles) << " nanoseconds\n";
  std::cout << "kstd::vector: " << (counter_kstd / cycles) << " nanoseconds\n";
  /*
  kstd::vector<bar> a;
  std::vector<bar> b = {bar("b1"), bar("b2"), bar("b3"), bar("b4"), bar("b5")};
  std::cout << "init done\n";
  a.reserve(4);
  a.emplace_back("a1");
  a.emplace_back("a2");
  a.emplace_back("a3");
  a.emplace_back("a4");
  std::cout << "inserting\n";
  a.insert(a.begin() + 2, 2, bar{"new"});
  std::cout << "done inserting\n";
  for (auto& i : a)
    std::cout << i.tag << std::endl;*/
  std::cin.ignore();
  return 0;
}