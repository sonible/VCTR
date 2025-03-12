cmake_minimum_required (VERSION 3.15)

project (VCTR_TEST)

# ================================================================================

find_package (Catch2 REQUIRED)
find_package (gcem REQUIRED)

if (TARGET gcem AND NOT TARGET gcem::gcem)
        add_library (gcem::gcem ALIAS gcem)
endif()

# ================================================================================
add_executable (vctr_test EXCLUDE_FROM_ALL vctr_test.cpp)

if (APPLE)
    target_compile_definitions (vctr_test PUBLIC -DACCELERATE_NEW_LAPACK)
endif()

target_link_libraries (vctr_test PRIVATE vctr vctr_test_utils vctr_test_cases Catch2::Catch2 gcem::gcem)
