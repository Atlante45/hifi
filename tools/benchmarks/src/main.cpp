//
//  main.cpp
//  tools/benchmarks/src
//
//  Created by Clément Brisset on 11/12/2015.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html

#include <benchmark.h>

#include <QSharedPointer>
#include <memory>

static void StdPointerCopy(benchmark::State& state) {
    using Pointer = std::shared_ptr<int>;
    Pointer p { new int(0) };
    
    while (state.KeepRunning()) {
        Pointer copy { p };
        benchmark::DoNotOptimize(copy);
    }
}
BENCHMARK(StdPointerCopy);

static void StdPointerRef(benchmark::State& state) {
    using Pointer = std::shared_ptr<int>;
    Pointer p { new int(0) };
    
    while (state.KeepRunning()) {
        Pointer& ref { p };
        benchmark::DoNotOptimize(ref);
    }
}
BENCHMARK(StdPointerRef);

static void QPointerCopy(benchmark::State& state) {
    using Pointer = QSharedPointer<int>;
    Pointer p { new int(0) };
    
    while (state.KeepRunning()) {
        Pointer copy { p };
        benchmark::DoNotOptimize(copy);
    }
}
BENCHMARK(QPointerCopy);

static void QPointerRef(benchmark::State& state) {
    using Pointer = QSharedPointer<int>;
    Pointer p { new int(0) };
    
    while (state.KeepRunning()) {
        Pointer& ref { p };
        benchmark::DoNotOptimize(ref);
    }
}
BENCHMARK(QPointerRef);

BENCHMARK_MAIN();
