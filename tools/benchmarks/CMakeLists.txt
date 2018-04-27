set(TARGET_NAME benchmarks)
setup_hifi_project()

link_hifi_libraries(shared)

set(BENCHMARK_LOCATION "/usr/local/Cellar/google-benchmark/1.4.0")

target_include_directories(${TARGET_NAME} PUBLIC "${BENCHMARK_LOCATION}/include/benchmark/")
target_link_libraries(${TARGET_NAME} "${BENCHMARK_LOCATION}/lib/libbenchmark.a")

