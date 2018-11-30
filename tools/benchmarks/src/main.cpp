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

//#include <QSharedPointer>
//#include <memory>
//#include <random>
#include <thread>

#include <Profile.h>


static void ProfileRange(benchmark::State& state) {
    if (state.thread_index == 0) {
        tracing::startTracing();
    }

    for (auto _ : state) {
        Duration profileRangeThis(tracing::app, "test");
        benchmark::DoNotOptimize(profileRangeThis);
    }

    if (state.thread_index == 0) {
        tracing::stopTracing();
    }
}
BENCHMARK(ProfileRange)->Threads(1);
BENCHMARK(ProfileRange)->Threads(2);
BENCHMARK(ProfileRange)->Threads(4);
BENCHMARK(ProfileRange)->Threads(8);
BENCHMARK(ProfileRange)->Threads(16);
BENCHMARK(ProfileRange)->Threads(32);
BENCHMARK(ProfileRange)->Threads(64);
BENCHMARK(ProfileRange)->Threads(72);

//static void StdPointerCopy(benchmark::State& state) {
//    using Pointer = std::shared_ptr<int>;
//    Pointer p { new int(0) };
//
//    while (state.KeepRunning()) {
//        Pointer copy { p };
//        benchmark::DoNotOptimize(copy);
//    }
//}
//BENCHMARK(StdPointerCopy);
//
//static void StdPointerRef(benchmark::State& state) {
//    using Pointer = std::shared_ptr<int>;
//    Pointer p { new int(0) };
//
//    while (state.KeepRunning()) {
//        Pointer& ref { p };
//        benchmark::DoNotOptimize(ref);
//    }
//}
//BENCHMARK(StdPointerRef);
//
//static void QPointerCopy(benchmark::State& state) {
//    using Pointer = QSharedPointer<int>;
//    Pointer p { new int(0) };
//
//    while (state.KeepRunning()) {
//        Pointer copy { p };
//        benchmark::DoNotOptimize(copy);
//    }
//}
//BENCHMARK(QPointerCopy);
//
//static void QPointerRef(benchmark::State& state) {
//    using Pointer = QSharedPointer<int>;
//    Pointer p { new int(0) };
//
//    while (state.KeepRunning()) {
//        Pointer& ref { p };
//        benchmark::DoNotOptimize(ref);
//    }
//}
//BENCHMARK(QPointerRef);
//
//static void RandomConstruction(benchmark::State& state) {
//    while (state.KeepRunning()) {
//        std::random_device randomDevice;
//        std::mt19937 generator(randomDevice());
//        std::uniform_real_distribution<float> distribution;
//        distribution.reset();
//        auto value = distribution(generator);
//        benchmark::DoNotOptimize(value);
//    }
//}
//BENCHMARK(RandomConstruction);
//
//static void RandomCall(benchmark::State& state) {
//    std::random_device randomDevice;
//    std::mt19937 generator(randomDevice());
//    std::uniform_real_distribution<float> distribution;
//
//    while (state.KeepRunning()) {
//        auto value = distribution(generator);
//        benchmark::DoNotOptimize(value);
//    }
//}
//BENCHMARK(RandomCall);

BENCHMARK_MAIN();
