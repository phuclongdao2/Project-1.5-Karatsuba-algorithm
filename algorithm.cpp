#include "header.hpp"

//Cộng hai đa thức A, B (deg A <= deg B)
void Add(const long long* a, int size_a, const long long* b, int size_b, long long* Result) {
    std::copy_n(b, size_b, Result);
    for (int i = 0; i < size_a; ++i) Result[i] += a[i];
}

//Thuật toán trực tiếp nhân hai đa thức A, B
void BruteForceMultiply(const long long* a, int size_a, const long long* b, int size_b, long long* Result) {
    for (int i = 0; i < size_a; ++i) {
        long long temp = a[i];
        for (int j = 0; j < size_b; ++j) Result[i + j] += temp * b[j];
    }
}

//Thuật toán Karatsuba song song nhân hai đa thức A, B có cùng bậc
void ParallelKaratsuba(const long long* a, const long long* b, int size, long long* Result, int depth) {
    if (size <= 100) BruteForceMultiply(a, size, b, size, Result);
    else {
        int mid = size >> 1, high = size - mid;
        int mid2 = mid << 1, highsize = (high << 1) - 1;
        //P1: Result[0...mid*2-2], P2 : Result[mid*2...size*2-2]
        Polynomial P3(highsize);
        Polynomial Q(highsize); //Q = P3 - P1 - P2
        Polynomial sumAsumB(highsize + 1);
        Add(a, mid, a + mid, high, sumAsumB.data());
        Add(b, mid, b + mid, high, &sumAsumB[high]);

        //Có sử dụng tính toán song song
        if (depth) {
            std::thread thread1([&] { ParallelKaratsuba(a, b, mid, Result, depth - 1); });
            std::thread thread2([&] { ParallelKaratsuba(a + mid, b + mid, high, Result + mid2, depth - 1); });
            ParallelKaratsuba(sumAsumB.data(), &sumAsumB[high], high, P3.data(), depth - 1);
            thread1.join(); thread2.join();
        }

        //Không sử dụng tính toán song song
        else {
            ParallelKaratsuba(a, b, mid, Result, 0);
            ParallelKaratsuba(a + mid, b + mid, high, Result + mid2, 0);
            ParallelKaratsuba(sumAsumB.data(), &sumAsumB[high], high, P3.data(), 0);
        }

        Result[mid2 - 1] = 0;
        int i = 0;
        for (; i < mid2 - 1; ++i) Q[i] = P3[i] - Result[i] - Result[i + mid2];
        if (size & 1) {
            Q[i] = P3[i] - Result[i + mid2];
            ++i;
            Q[i] = P3[i] - Result[i + mid2];
        }
        for (i = 0; i < highsize; ++i) Result[i + mid] += Q[i];
    }
}

//Quy về hai đa thức có bậc bằng nhau
void PrepareKaratsuba(const Polynomial& A, const Polynomial& B, Polynomial& Result, int depth) {
    int size = std::max(A.size(), B.size());
    Result.resize((size << 1) - 1);
    if (A.size() < B.size()) {
        Polynomial A_resized(size, 0);
        std::copy(A.begin(), A.end(), A_resized.begin());
        ParallelKaratsuba(A_resized.data(), B.data(), size, Result.data(), depth);
    }
    else if (A.size() > B.size()) {
        Polynomial B_resized(size, 0);
        std::copy(B.begin(), B.end(), B_resized.begin());
        ParallelKaratsuba(A.data(), B_resized.data(), size, Result.data(), depth);
    }
    else ParallelKaratsuba(A.data(), B.data(), size, Result.data(), depth);
}
