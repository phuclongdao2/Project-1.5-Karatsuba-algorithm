#include "header.hpp"

//Cộng hai đa thức A, B (deg A <= deg B)
void Add(const long long* a, int size_a, const long long* b, int size_b, long long* Result) {
    std::copy_n(b, size_b, Result);
    std::transform(a, a + size_a, Result, Result, std::plus<>());
}

//Thuật toán trực tiếp nhân hai đa thức A, B
void BruteForceMultiply(const long long* a, int size_a, const long long* b, int size_b, long long* Result) {
    for (int i = 0; i < size_a; ++i) {
        long long temp = a[i];
        for (int j = 0; j < size_b; ++j) Result[i + j] += temp * b[j];
    }
}

//Thuật toán Karatsuba song song nhân hai đa thức A, B có cùng bậc
void ParallelKaratsuba(const long long* a, const long long* b, int size, Polynomial& Result, int depth) {
    if (size <= 100) BruteForceMultiply(a, size, b, size, Result.data());
    else {
        int mid = size >> 1, high = size - mid;
        int midsize = (mid << 1) - 1, highsize = (high << 1) - 1;
        Polynomial P1(midsize);
        Polynomial P2(highsize);
        Polynomial P3(highsize);
        Polynomial sumAsumB(high * 2);
        Add(a, mid, a + mid, high, sumAsumB.data());
        Add(b, mid, b + mid, high, &sumAsumB[high]);

        //Có sử dụng tính toán song song
        if (depth) {
            std::thread thread1([&] { ParallelKaratsuba(a, b, mid, P1, depth - 1); });
            std::thread thread2([&] { ParallelKaratsuba(a + mid, b + mid, high, P2, depth - 1); });
            ParallelKaratsuba(sumAsumB.data(), &sumAsumB[high], high, P3, depth - 1);
            thread1.join(); thread2.join();
        }

        //Không sử dụng tính toán song song
        else {
            ParallelKaratsuba(a, b, mid, P1, 0);
            ParallelKaratsuba(a + mid, b + mid, high, P2, 0);
            ParallelKaratsuba(sumAsumB.data(), &sumAsumB[high], high, P3, 0);
        }

        std::copy_n(P1.begin(), midsize, Result.begin());
        std::copy_n(P2.begin(), highsize, Result.begin() + mid * 2);
        Result[midsize] = 0;
        int i = 0;
        for (; i < midsize; ++i) Result[i + mid] += P3[i] - P1[i] - P2[i];
        if (size & 1) {
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
        Polynomial A_resized(size, 0);
        std::copy(A.begin(), A.end(), A_resized.begin());
        ParallelKaratsuba(A_resized.data(), B.data(), size, Result, depth);
    }
    else if (A.size() > B.size()) {
        Polynomial B_resized(size, 0);
        std::copy(B.begin(), B.end(), B_resized.begin());
        ParallelKaratsuba(A.data(), B_resized.data(), size, Result, depth);
    }
    else ParallelKaratsuba(A.data(), B.data(), size, Result, depth);
}
