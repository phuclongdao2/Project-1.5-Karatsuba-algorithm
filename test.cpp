#include "header.hpp"
const int NUM_TESTS = 10, MAX_PARALLEL_DEPTH = 5;
Polynomial P, Q;

int main() {

    //Tạo ngẫu nhiên hai đa thức P, Q có bậc là deg
    int deg; std::cin >> deg;
    std::mt19937 gen(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> dist(-1000, 1000);
    for (int i = 0; i <= deg; ++i) P.push_back(dist(gen));
    for (int i = 0; i <= deg; ++i) Q.push_back(dist(gen));

    //Tính P*Q bằng thuật toán trực tiếp (để kiểm tra kết quả)
    Polynomial BruteForceAns((deg << 1) + 1, 0);
    BruteForceMultiply(&P[0], deg + 1, &Q[0], deg + 1, BruteForceAns.data());

    //Tính P*Q bằng Karatsuba với depth lần lượt là 0, 1, ..., MAX_PARALLEL_DEPTH (depth = 0: không dùng tính toán song song)
    std::vector <double> time_taken[MAX_PARALLEL_DEPTH + 1];
    for (int test = 1; test <= NUM_TESTS; ++test) {
        Polynomial ans[MAX_PARALLEL_DEPTH + 1];
        printf("Test %d:\n", test);
        for (int depth = 0; depth <= MAX_PARALLEL_DEPTH; ++depth) {
            clock_t start = clock();
            PrepareKaratsuba(P, Q, ans[depth], depth);
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
        printf("Depth %d: %.0f ms.\n", depth, sum / (NUM_TESTS - NUM_TESTS / 2));
    }
}
