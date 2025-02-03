#pragma once

#include <chrono>
#include <iostream>
#include <numeric>
#include <random>
#include <thread>
#include <vector>
using Polynomial = std::vector<long long>;

Polynomial Add(const Polynomial& A, const Polynomial& B);
Polynomial BruteForceMultiply(const Polynomial& A, const Polynomial& B);
void ParallelKaratsuba(const Polynomial& A, const Polynomial& B, Polynomial& Result, int depth);