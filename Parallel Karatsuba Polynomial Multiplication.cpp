#include <chrono>
#include <iostream>
#include <numeric>
#include <random>
#include <thread>
#include <vector>
using Polynomial = std::vector<long long>;
const int NUM_TESTS = 10, MAX_PARALLEL_DEPTH = 5;
Polynomial P, Q;

//Cộng hai đa thức A, B
Polynomial Add(const Polynomial& A, const Polynomial& B) {
    Polynomial Result(std::max(A.size(), B.size()), 0);
    for (int i = 0; i < A.size(); ++i) Result[i] += A[i];
    for (int i = 0; i < B.size(); ++i) Result[i] += B[i];
    return Result;
}

//Thuật toán trực tiếp nhân hai đa thức A, B
Polynomial BruteForceMultiply(const Polynomial& A, const Polynomial& B) {
    Polynomial Result(A.size() + B.size() - 1, 0);
    for (int i = 0; i < A.size(); ++i) {
        long long temp = A[i];
        for (int j = 0; j < B.size(); ++j) Result[i + j] += temp * B[j];
    }
    return Result;
}

//Thuật toán Karatsuba song song nhân hai đa thức A, B
void ParallelKaratsuba(const Polynomial& A, const Polynomial& B, Polynomial& Result, int depth) {
    int n = std::max(A.size(), B.size());
    Polynomial A_resized(A), B_resized(B);
    A_resized.resize(n, 0); B_resized.resize(n, 0);
    Result.resize((n << 1) - 1, 0);
    if (n <= 50) Result = BruteForceMultiply(A, B);
    else {
        int mid = n >> 1;
        Polynomial A_low(A_resized.begin(), A_resized.begin() + mid);
        Polynomial A_high(A_resized.begin() + mid, A_resized.end());
        Polynomial B_low(B_resized.begin(), B_resized.begin() + mid);
        Polynomial B_high(B_resized.begin() + mid, B_resized.end());
        Polynomial P1(A_low.size() + B_low.size() - 1, 0);
        Polynomial P2(A_high.size() + B_high.size() - 1, 0);
        Polynomial P3(A_high.size() + B_high.size() - 1, 0);

        //Có sử dụng tính toán song song
        if (depth) {
            std::thread thread1(ParallelKaratsuba, std::ref(A_low), std::ref(B_low), std::ref(P1), depth - 1);
            std::thread thread2(ParallelKaratsuba, std::ref(A_high), std::ref(B_high), std::ref(P2), depth - 1);
            ParallelKaratsuba(Add(A_low, A_high), Add(B_low, B_high), P3, depth - 1);
            thread1.join(); thread2.join();
        }

        //Không sử dụng tính toán song song
        else {
            ParallelKaratsuba(A_low, B_low, P1, 0);
            ParallelKaratsuba(A_high, B_high, P2, 0);
            ParallelKaratsuba(Add(A_low, A_high), Add(B_low, B_high), P3, 0);
        }
        for (int i = 0; i < P1.size(); ++i) Result[i] = P1[i];
        for (int i = 0; i < P2.size(); ++i) Result[i + (mid << 1)] += P2[i];
        for (int i = 0; i < P3.size(); ++i) {
            Result[i + mid] += P3[i] - P2[i];
            if (i < P1.size()) Result[i + mid] -= P1[i];
        }
    }
}

int main() {

    //Tạo ngẫu nhiên hai đa thức P, Q có bậc là deg
    int deg; std::cin >> deg;
    std::mt19937 gen(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> dist(-1000, 1000);
    for (int i = 0; i <= deg; ++i) P.push_back(dist(gen));
    for (int i = 0; i <= deg; ++i) Q.push_back(dist(gen));

    //Tính P*Q bằng thuật toán trực tiếp (để kiểm tra kết quả)
    Polynomial BruteForceAns = BruteForceMultiply(P, Q);

    //Tính P*Q bằng Karatsuba với depth lần lượt là 0, 1, ..., MAX_PARALLEL_DEPTH (depth = 0: không dùng tính toán song song)
    std::vector <double> time_taken[MAX_PARALLEL_DEPTH + 1];
    for (int test = 1; test <= NUM_TESTS; ++test) {
        Polynomial ans[MAX_PARALLEL_DEPTH + 1];
        printf("Test %d:\n", test);
        for (int depth = 0; depth <= MAX_PARALLEL_DEPTH; ++depth) {
            clock_t start = clock();
            ParallelKaratsuba(P, Q, ans[depth], depth);
            clock_t finish = clock();
            double taken = (double)(finish - start) / CLOCKS_PER_SEC * 1000;
            if (test > NUM_TESTS / 2) time_taken[depth].push_back(taken);

            //Kiểm tra kết quả
            printf("Parallel Karatsuba depth %d: %.0f ms ", depth, taken);
            printf(BruteForceAns == ans[depth] ? "Correct answer!\n" : "Wrong answer!\n");
        }
    }

    //Tính thời gian chạy trung bình
    printf("Average:\n");
    for (int depth = 0; depth <= MAX_PARALLEL_DEPTH; ++depth) {
        double sum = 0;
        for (auto t : time_taken[depth]) sum += t;
        printf("Depth %d: %.0f ms.\n", depth, sum / (NUM_TESTS / 2));
    }
}