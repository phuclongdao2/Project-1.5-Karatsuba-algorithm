#include "header.hpp"

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
    if (n <= 50) Result = BruteForceMultiply(A, B);
    else {
        //Quy về hai đa thức có bậc bằng nhau
        if (A.size() < B.size()) {
            Polynomial A_resized(A);
            A_resized.resize(n, 0);
            ParallelKaratsuba(A_resized, B, Result, depth);
        }
        else if (A.size() > B.size()) {
            Polynomial B_resized(B);
            B_resized.resize(n, 0);
            ParallelKaratsuba(A, B_resized, Result, depth);
        }
        int mid = n >> 1;
        Result.resize((n << 1) - 1, 0);
        Polynomial A_low(A.begin(), A.begin() + mid);
        Polynomial A_high(A.begin() + mid, A.end());
        Polynomial B_low(B.begin(), B.begin() + mid);
        Polynomial B_high(B.begin() + mid, B.end());
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