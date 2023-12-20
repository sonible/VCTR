cmake_minimum_required (VERSION 3.15)
project (VCTR VERSION 0.0.1)

include (CMakeDependentOption)
option (VCTR_AUTOLINK_IPP       "Use and automatically link IPP on supported platforms" OFF)
option (VCTR_USE_CONAN          "Use conan to install dependencies"                     OFF)
option (VCTR_BUILD_TEST         "Build unit tests"                                      OFF)
option (VCTR_BUILD_DEMO         "Build demo project"                                    OFF)
option (VCTR_BUILD_BENCHMARK    "Build benchmark project"                               OFF)

set (VCTR_IPP_ROOT "" CACHE STRING "Custom search path for IPP")

add_library (vctr INTERFACE)

target_compile_features (vctr INTERFACE cxx_std_20)
target_include_directories (vctr INTERFACE include)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
if (VCTR_AUTOLINK_IPP)
    find_package (IPP REQUIRED)
    target_link_libraries (vctr INTERFACE ipp)
    target_compile_definitions (vctr INTERFACE VCTR_USE_IPP=1)
endif()

if (VCTR_USE_CONAN)
    if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
        # latest version 0.18.1 does not detect clang + windows setup
        # see: https://github.com/conan-io/cmake-conan/issues/459
        file (DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.17.0/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake" TLS_VERIFY ON)
    endif()
    include (${CMAKE_BINARY_DIR}/conan.cmake)
endif()

if (APPLE)
    target_link_libraries (vctr INTERFACE "-framework Accelerate")
endif()

# clang-cl needs special handling as it disables exceptions by default
if (CMAKE_CXX_COMPILER_ID AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT AND CMAKE_CXX_SIMULATE_ID)
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL Clang AND ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT} STREQUAL MSVC AND ${CMAKE_CXX_SIMULATE_ID} STREQUAL MSVC)
        target_compile_options (vctr INTERFACE /EHa)
    endif()
endif()

if (VCTR_EXPOSE_UNIT_TEST_CASES_TARGET OR VCTR_BUILD_TEST)
    add_subdirectory (test)
endif()

add_library (sonible::vctr ALIAS vctr)

if (VCTR_BUILD_BENCHMARK)
    add_subdirectory (benchmark)
endif()

if (VCTR_BUILD_DEMO)
    add_subdirectory (demo)
endif()
