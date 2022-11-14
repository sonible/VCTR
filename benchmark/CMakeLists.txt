cmake_minimum_required (VERSION 3.15)

project (VCTR_BENCHMARK)

# ================================================================================
add_executable (vctr_benchmark EXCLUDE_FROM_ALL benchmark.cpp)

if (VCTR_USE_CONAN)
    conan_cmake_configure (REQUIRES catch2/3.1.0 BUILD missing GENERATORS cmake_find_package)
    conan_cmake_autodetect (settings)
    conan_cmake_install (PATH_OR_REFERENCE . BUILD missing SETTINGS ${settings})
    set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_BINARY_DIR})
endif()

find_package (Catch2 REQUIRED)
target_link_libraries (vctr_benchmark
        PRIVATE
            vctr
            ipp
            Catch2::Catch2)
