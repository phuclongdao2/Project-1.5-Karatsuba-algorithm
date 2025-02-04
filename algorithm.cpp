#include "header.hpp"

//Cộng hai đa thức A, B
Polynomial Add(const long long* a, int size_a, const long long* b, int size_b) {
    if (size_a > size_b) {
        Polynomial Result(size_a);
        std::copy_n(a, size_a, Result.begin());
        for (int i = 0; i < size_b; ++i) Result[i] += b[i];
        return Result;
    }
    else {
        Polynomial Result(size_b);
        std::copy_n(b, size_b, Result.begin());
        for (int i = 0; i < size_a; ++i) Result[i] += a[i];
        return Result;
    }
}

//Thuật toán trực tiếp nhân hai đa thức A, B
Polynomial BruteForceMultiply(const long long* a, int size_a, const long long* b, int size_b) {
    Polynomial Result(size_a + size_b - 1, 0);
    for (int i = 0; i < size_a; ++i) {
        long long temp = a[i];
        for (int j = 0; j < size_b; ++j) Result[i + j] += temp * b[j];
    }
    return Result;
}

//Thuật toán Karatsuba song song nhân hai đa thức A, B có cùng bậc
void ParallelKaratsuba(const long long* a, const long long* b, int size, Polynomial& Result, int depth) {
    if (size <= 100) Result = BruteForceMultiply(a, size, b, size);
    else {
        int mid = size >> 1, high = size - mid;
        int midsize = (mid << 1) - 1, highsize = (high << 1) - 1;
        Polynomial P1(midsize);
        Polynomial P2(highsize);
        Polynomial P3(highsize);
        Polynomial sumA = Add(a, mid, a + mid, high);
        Polynomial sumB = Add(b, mid, b + mid, high);

        //Có sử dụng tính toán song song
        if (depth) {
            std::thread thread1(ParallelKaratsuba, a, b, mid, std::ref(P1), depth - 1);
            std::thread thread2(ParallelKaratsuba, a + mid, b + mid, high, std::ref(P2), depth - 1);
            ParallelKaratsuba(&sumA[0], &sumB[0], high, P3, depth - 1);
            thread1.join(); thread2.join();
        }

        //Không sử dụng tính toán song song
        else {
            ParallelKaratsuba(a, b, mid, P1, 0);
            ParallelKaratsuba(a + mid, b + mid, high, P2, 0);
            ParallelKaratsuba(&sumA[0], &sumB[0], high, P3, 0);
        }

        std::copy_n(&P1[0], midsize, Result.begin());
        std::copy_n(&P2[0], highsize, Result.begin() + mid * 2);
        Result[midsize] = 0;
        int i = 0;
        for (; i < midsize; ++i) Result[i + mid] += P3[i] - P1[i] - P2[i];
        if (size % 2) {
            Result[i + mid] += P3[i] - P2[i];
            ++i;
            Result[i + mid] += P3[i] - P2[i];
        }
    }
}

//Quy về hai đa thức có bậc bằng nhau
void PrepareKaratsuba(const Polynomial& A, const Polynomial& B, Polynomial& Result, int depth) {
    int size = std::max(A.size(), B.size());
    Result.resize((size << 1) - 1);
    if (A.size() < B.size()) {
        Polynomial A_resized(A);
        A_resized.resize(size, 0);
        ParallelKaratsuba(&A_resized[0], &B[0], size, Result, depth);
    }
    else if (A.size() > B.size()) {
        Polynomial B_resized(B);
        B_resized.resize(size, 0);
        ParallelKaratsuba(&A[0], &B_resized[0], size, Result, depth);
    }
    else ParallelKaratsuba(&A[0], &B[0], size, Result, depth);
}
