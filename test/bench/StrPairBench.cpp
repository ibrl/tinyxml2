#include <benchmark/benchmark.h>
#include <cstring>
#include <assert.h>
#include <climits>
#include <charconv>
#include <iostream>

#define SEMICOLON ';'
#define TIXMLASSERT assert

template <int BASE>
unsigned long checkIntegerValueNew(const char* p)
{
    unsigned long ucs {0};
    if (p == nullptr) {
        return 0;
    }
    if (*(p+2) == 'x') {
        const char * start = p+3;
        if (start) {
            const char * endp  = strchr(start, SEMICOLON);
            if (endp) {
                auto [ptr, ec] { std::from_chars(start, endp, ucs, BASE) };
                if (ec == std::errc()) {
                    return ucs;
                }
            }
        }
    }
    return 0;        
}

unsigned long checkIntegerValue(const char* p )
{
    ptrdiff_t delta = 0;
    unsigned mult = 1;
    unsigned long ucs = 0;

    if (p == nullptr) {
        return ucs;
    }
    
    if ( *(p+2) == 'x' ) {
        // Hexadecimal.
        const char* q = p+3;
        if ( !(*q) ) {
            return 0;
        }

        q = strchr( q, SEMICOLON );

        if ( !q ) {
            return 0;
        }
        TIXMLASSERT( *q == SEMICOLON );

        delta = q-p;
        --q;

        while ( *q != 'x' ) {
            unsigned int digit = 0;

            if ( *q >= '0' && *q <= '9' ) {
                digit = *q - '0';
            }
            else if ( *q >= 'a' && *q <= 'f' ) {
                digit = *q - 'a' + 10;
            }
            else if ( *q >= 'A' && *q <= 'F' ) {
                digit = *q - 'A' + 10;
            }
            else {
                return 0;
            }
            TIXMLASSERT( digit < 16 );
            TIXMLASSERT( digit == 0 || mult <= UINT_MAX / digit );
            const unsigned int digitScaled = mult * digit;
            TIXMLASSERT( ucs <= ULONG_MAX - digitScaled );
            ucs += digitScaled;
            TIXMLASSERT( mult <= UINT_MAX / 16 );
            mult *= 16;
            --q;
        }
    }
    return ucs;
}



// Define another benchmark
static void BM_CheckIntegerValue(benchmark::State& state) {
    const char* testString1 = "&#x4acD;dfjk";

    for (auto _ : state) {
        benchmark::DoNotOptimize(checkIntegerValue(testString1));
    }
}

static void BM_CheckIntegerValue1(benchmark::State& state) {
    const char* testString1 = "&#x4L";

    for (auto _ : state) {
        benchmark::DoNotOptimize(checkIntegerValue(testString1));
    }
}

static void BM_CheckIntegerValue2(benchmark::State& state) {
    const char* testString1 = nullptr;

    for (auto _ : state) {
        benchmark::DoNotOptimize(checkIntegerValue(testString1));
    }
}

static void BM_CheckIntegerValueNew(benchmark::State& state) {
    const char* testString1 = "&#x4acD;dfjk";

    for (auto _ : state) {
        benchmark::DoNotOptimize(checkIntegerValueNew<16>(testString1));
    }
}

static void BM_CheckIntegerValueNew1(benchmark::State& state) {
    const char* testString1 = "&#x4L";

    for (auto _ : state) {
        benchmark::DoNotOptimize(checkIntegerValueNew<16>(testString1));
    }
}

static void BM_CheckIntegerValueNew2(benchmark::State& state) {
    const char* testString1 = nullptr;

    for (auto _ : state) {
        benchmark::DoNotOptimize(checkIntegerValueNew<16>(testString1));
    }
}

static void BM_CheckIntegerValueNew3(benchmark::State& state) {
    const char* testString1 = "&#xffffffffffffffff;as";

    for (auto _ : state) {
        benchmark::DoNotOptimize(checkIntegerValueNew<16>(testString1));
    }
}


BENCHMARK(BM_CheckIntegerValue);
BENCHMARK(BM_CheckIntegerValue1);
BENCHMARK(BM_CheckIntegerValue2);
BENCHMARK(BM_CheckIntegerValueNew);
BENCHMARK(BM_CheckIntegerValueNew1);
BENCHMARK(BM_CheckIntegerValueNew2);
BENCHMARK(BM_CheckIntegerValueNew3);

BENCHMARK_MAIN();
