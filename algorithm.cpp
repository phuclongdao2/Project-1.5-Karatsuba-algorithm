#include "header.hpp"

//Cộng hai đa thức A, B (deg A <= deg B)
void Add(const long long* a, int size_a, const long long* b, int size_b, long long* result) {
    std::copy_n(b, size_b, result);
    for (int i = 0; i < size_a; ++i) result[i] += a[i];
}

//Thuật toán trực tiếp nhân hai đa thức A, B
void BruteForceMultiply(const long long* a, int size_a, const long long* b, int size_b, long long* result) {
    for (int i = 0; i < size_a; ++i) {
        long long temp = a[i];
        for (int j = 0; j < size_b; ++j) result[j] += temp * b[j];
        ++result;
    }
}

//Thuật toán Karatsuba song song nhân hai đa thức A, B có cùng bậc
void ParallelKaratsuba(const long long* a, const long long* b, int size, long long* result, int depth) {
    if (size <= 100) BruteForceMultiply(a, size, b, size, result);
    else {
        //P1: Result[0 ... mid * 2 - 2], P2 : Result[mid * 2 ... size * 2 - 2]
        //sumA: P3[high * 2 - 1 ... high * 3 - 2], sumB: P3[high * 3 - 1 ... high * 4 - 2]
        int mid = size >> 1, high = size - mid, mid2 = mid << 1, midsize = mid2 - 1;
        long long* resmid = result + mid, * P2 = result + mid2;
        Polynomial P3((high << 2) - 1);
        long long* sumA = P3.data() + (high << 1) - 1, * sumB = sumA + high;
        Add(a, mid, a + mid, high, sumA);
        Add(b, mid, b + mid, high, sumB);

        //Có sử dụng tính toán song song
        if (depth) {
            std::thread thread1([&] { ParallelKaratsuba(a, b, mid, result, depth - 1); });
            std::thread thread2([&] { ParallelKaratsuba(a + mid, b + mid, high, P2, depth - 1); });
            ParallelKaratsuba(sumA, sumB, high, P3.data(), depth - 1);
            thread1.join(); thread2.join();
        }

        //Không sử dụng tính toán song song
        else {
            ParallelKaratsuba(a, b, mid, result, 0);
            ParallelKaratsuba(a + mid, b + mid, high, P2, 0);
            ParallelKaratsuba(sumA, sumB, high, P3.data(), 0);
        }

        result[midsize] = 0;
        int i = 0;
        for (; i < mid; ++i) resmid[i] += (P3[i] -= result[i]) - P2[i];
        for (; i < midsize; ++i) resmid[i] = P3[i - mid] + P3[i] - result[i] - P2[i];
        if (size & 1) {
            resmid[midsize] += P3[midsize] - P2[midsize];
            resmid[mid2] += P3[mid2] - P2[mid2];
        }
    }
}

//Quy về hai đa thức có bậc bằng nhau
void PrepareKaratsuba(const Polynomial& A, const Polynomial& B, Polynomial& Result, int depth) {
    int size = std::max(A.size(), B.size());
    Result.resize((size << 1) - 1);
    if (A.size() < B.size()) {
        Polynomial A_resized = A;
        A_resized.resize(size, 0);
        ParallelKaratsuba(A_resized.data(), B.data(), size, Result.data(), depth);
    }
    else if (A.size() > B.size()) {
        Polynomial B_resized = B;
        B_resized.resize(size, 0);
        ParallelKaratsuba(A.data(), B_resized.data(), size, Result.data(), depth);
    }
    else ParallelKaratsuba(A.data(), B.data(), size, Result.data(), depth);
    Result.resize(A.size() + B.size() - 1);
}
