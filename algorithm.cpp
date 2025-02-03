#include "header.hpp"

//Cộng hai đa thức A, B
Polynomial Add(const long long* a, int size_a, const long long* b, int size_b) {
    if (size_a > size_b) {
        Polynomial Result = { a, a + size_a };
        for (int i = 0; i < size_b; ++i) Result[i] += b[i];
        return Result;
    }
    else {
        Polynomial Result = { b, b + size_b };
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
void ParallelKaratsuba(const long long *a, const long long *b, int size, Polynomial& Result, int depth) {
    if (size <= 50) Result = BruteForceMultiply(a, size, b, size);
    else {
        int mid = size >> 1;
        Result.resize((size << 1) - 1, 0);
        Polynomial P1((mid << 1) - 1, 0);
        Polynomial P2(((size - mid) << 1) - 1, 0);
        Polynomial P3(((size - mid) << 1) - 1, 0);
        Polynomial sumA = Add(a, mid, a + mid, size - mid);
        Polynomial sumB = Add(b, mid, b + mid, size - mid);

        //Có sử dụng tính toán song song
        if (depth) {
            std::thread thread1(ParallelKaratsuba, a, b, mid, std::ref(P1), depth - 1);
            std::thread thread2(ParallelKaratsuba, a + mid, b + mid, size - mid, std::ref(P2), depth - 1);
            ParallelKaratsuba(&sumA[0], &sumB[0], size - mid, P3, depth - 1);
            thread1.join(); thread2.join();
        }

        //Không sử dụng tính toán song song
        else {
            ParallelKaratsuba(a, b, mid, P1, 0);
            ParallelKaratsuba(a + mid, b + mid, size - mid, P2, 0);
            ParallelKaratsuba(&sumA[0], &sumB[0], size - mid, P3, 0);
        }
        for (int i = 0; i < P1.size(); ++i) Result[i] = P1[i];
        for (int i = 0; i < P2.size(); ++i) Result[i + (mid << 1)] += P2[i];
        for (int i = 0; i < P3.size(); ++i) {
            Result[i + mid] += P3[i] - P2[i];
            if (i < P1.size()) Result[i + mid] -= P1[i];
        }
    }
}

//Quy về hai đa thức có bậc bằng nhau
void PrepareKaratsuba(const Polynomial& A, const Polynomial& B, Polynomial& Result, int depth) {
    int n = std::max(A.size(), B.size());
    if (A.size() < B.size()) {
        Polynomial A_resized(A);
        A_resized.resize(n, 0);
        ParallelKaratsuba(&A_resized[0], &B[0], n, Result, depth);
    }
    else if (A.size() > B.size()) {
        Polynomial B_resized(B);
        B_resized.resize(n, 0);
        ParallelKaratsuba(&A[0], &B_resized[0], n, Result, depth);
    }
    else ParallelKaratsuba(&A[0], &B[0], n, Result, depth);
}
