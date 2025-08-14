#include <benchmark/benchmark.h>
#include <cmath>

// Dummy function to benchmark
static double ComputeSomething(int n)
{
    double result = 0.0;
    for (int i = 1; i <= n; ++i) {
        result += std::sqrt(static_cast<double>(i));
    }
    return result;
}

// Google Benchmark wrapper
static void BM_ComputeSomething(benchmark::State& state)
{
    for (auto _ : state) {
        benchmark::DoNotOptimize(ComputeSomething(static_cast<int>(state.range(0))));
    }
}

// Register with different input sizes
BENCHMARK(BM_ComputeSomething)->Arg(1000)->Arg(10000)->Arg(100000);

BENCHMARK_MAIN();
