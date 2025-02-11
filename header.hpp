#pragma once

#include <chrono>
#include <iostream>
#include <numeric>
#include <random>
#include <thread>
#include <vector>
using Polynomial = std::vector<long long>;

void Add(const long long* a, int size_a, const long long* b, int size_b, long long* Result);
Polynomial BruteForceMultiply(const long long* a, int size_a, const long long* b, int size_b);
void ParallelKaratsuba(const long long* a, const long long* b, int size, Polynomial& Result, int depth);
void PrepareKaratsuba(const Polynomial& A, const Polynomial& B, Polynomial& Result, int depth);
