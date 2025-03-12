cmake_minimum_required (VERSION 3.15)

project (VCTR_BENCHMARK)

# ================================================================================
add_executable (vctr_benchmark EXCLUDE_FROM_ALL benchmark.cpp)

find_package (Catch2 REQUIRED)
target_link_libraries (vctr_benchmark
        PRIVATE
            vctr
            ipp
            Catch2::Catch2)
