#pragma once
#include <chrono>
#include <iostream>
#include <numeric>
#include <random>
#include <thread>
#include <vector>
using Polynomial = std::vector<long long>;
using uint = unsigned int;

//Thuật toán trực tiếp nhân hai đa thức A, B
template <typename T> void BruteForceMultiply(const T* a, uint size_a, const T* b, uint size_b, T* result) {
    for (uint i = 0; i < size_a; ++i) {
        T temp = a[i];
        for (uint j = 0; j < size_b; ++j) result[j] += temp * b[j];
        ++result;
    }
}

//Thuật toán Karatsuba song song nhân hai đa thức A, B có cùng bậc
template <typename T> void ParallelKaratsuba(const T* a, const T* b, uint size, T* result, uint depth) {
    if (size <= 64) BruteForceMultiply(a, size, b, size, result);
    else {
        //P1: Result[0 ... mid * 2 - 2], P2 : Result[mid * 2 ... size * 2 - 2]
        //sumA: P3[high * 2 - 1 ... high * 3 - 2], sumB: P3[high * 3 - 1 ... high * 4 - 2]
        uint mid = size >> 1, high = size - mid, mid2 = mid << 1, midsize = mid2 - 1;
        T* resmid = result + mid, * P2 = result + mid2;
        T* P3 = new T[(high << 2) - 1]();
        T* sumA = P3 + (high << 1) - 1, * sumB = sumA + high;
        const T* amid = a + mid, * bmid = b + mid;
        uint i = 0;
        for (; i < mid; ++i) {
            sumA[i] = a[i] + amid[i];
            sumB[i] = b[i] + bmid[i];
        }
        if (size & 1) {
            sumA[mid] = amid[mid];
            sumB[mid] = bmid[mid];
        }

        //Có sử dụng tính toán song song
        if (depth) {
            std::thread thread1([&] { ParallelKaratsuba(a, b, mid, result, depth - 1); });
            std::thread thread2([&] { ParallelKaratsuba(amid, bmid, high, P2, depth - 1); });
            ParallelKaratsuba(sumA, sumB, high, P3, depth - 1);
            thread1.join(); thread2.join();
        }

        //Không sử dụng tính toán song song
        else {
            ParallelKaratsuba(a, b, mid, result, 0);
            ParallelKaratsuba(amid, bmid, high, P2, 0);
            ParallelKaratsuba(sumA, sumB, high, P3, 0);
        }

        result[midsize] = 0;
        for (i = 0; i < mid; ++i) resmid[i] += (P3[i] -= result[i]) - P2[i];
        for (; i < midsize; ++i) resmid[i] = P3[i - mid] + P3[i] - result[i] - P2[i];
        if (size & 1) {
            resmid[midsize] += P3[midsize] - P2[midsize];
            resmid[mid2] += P3[mid2] - P2[mid2];
        }
        delete[] P3;
    }
}

//Quy về hai đa thức có bậc bằng nhau
template <typename T> void PrepareKaratsuba(const T* a, uint size_a, const T* b, uint size_b, T* result, uint depth) {
    uint size = std::max(size_a, size_b), final_size = size_a + size_b - 1;
    T* temp = new T[(size << 1) - 1]();
    if (size_a < size_b) {
        T* a_resized = new T[size]();
        for (uint i = 0; i < size_a; ++i) a_resized[i] = a[i];
        ParallelKaratsuba(a_resized, b, size, temp, depth);
        delete[] a_resized;
    }
    else if (size_a > size_b) {
        T* b_resized = new T[size]();
        for (uint i = 0; i < size_b; ++i) b_resized[i] = b[i];
        ParallelKaratsuba(a, b_resized, size, temp, depth);
        delete[] b_resized;
    }
    else ParallelKaratsuba(a, b, size, temp, depth);
    for (uint i = 0; i < final_size; ++i) result[i] = temp[i];
    delete[] temp;
}

//Đánh giá hiệu năng
void Test(uint deg1, uint deg2) {
    //Tạo ngẫu nhiên hai đa thức P, Q
    const uint NUM_TESTS = 10, MAX_PARALLEL_DEPTH = 5;
    Polynomial P, Q;
    std::mt19937 gen(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> dist(-1000, 1000);
    for (uint i = 0; i <= deg1; ++i) P.push_back(dist(gen));
    for (uint i = 0; i <= deg2; ++i) Q.push_back(dist(gen));

    //Tính P*Q bằng thuật toán trực tiếp (để kiểm tra kết quả)
    Polynomial BruteForceAns(deg1 + deg2 + 1, 0);
    BruteForceMultiply(P.data(), deg1 + 1, Q.data(), deg2 + 1, BruteForceAns.data());

    //Tính P*Q bằng Karatsuba với depth lần lượt là 0, 1, ..., MAX_PARALLEL_DEPTH (depth = 0: không dùng tính toán song song)
    std::vector <double> time_taken[MAX_PARALLEL_DEPTH + 1];
    for (uint test = 1; test <= NUM_TESTS; ++test) {
        printf("Test %d:\n", test);
        for (uint depth = 0; depth <= MAX_PARALLEL_DEPTH; ++depth) {
            Polynomial ans(deg1 + deg2 + 1);
            clock_t start = clock();
            PrepareKaratsuba(P.data(), deg1 + 1, Q.data(), deg2 + 1, ans.data(), depth);
            clock_t finish = clock();
            double taken = (double)(finish - start) / CLOCKS_PER_SEC * 1000;
            if (test > NUM_TESTS / 2) time_taken[depth].push_back(taken);

            //Kiểm tra kết quả
            printf("Parallel Karatsuba depth %d: %.0f ms ", depth, taken);
            printf(BruteForceAns == ans ? "Correct answer!\n" : "Wrong answer!\n");
        }
    }

    //Tính thời gian chạy trung bình
    printf("Average:\n");
    for (uint depth = 0; depth <= MAX_PARALLEL_DEPTH; ++depth) {
        double sum = 0;
        for (auto t : time_taken[depth]) sum += t;
        printf("Depth %d: %.0f ms.\n", depth, sum / (NUM_TESTS - NUM_TESTS / 2));
    }
}
