cmake_minimum_required (VERSION 3.15)

project (VCTR_TEST)

# ================================================================================

if (VCTR_USE_CONAN)
        conan_cmake_configure (REQUIRES catch2/3.1.0 gcem/1.16.0 BUILD missing GENERATORS cmake_find_package)
        conan_cmake_autodetect (settings)
        conan_cmake_install (PATH_OR_REFERENCE . BUILD missing SETTINGS ${settings})
        set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_BINARY_DIR})
endif()
find_package (Catch2 REQUIRED)
find_package (gcem REQUIRED)

if (TARGET gcem AND NOT TARGET gcem::gcem)
        add_library (gcem::gcem ALIAS gcem)
endif()

# ================================================================================
add_executable (vctr_test EXCLUDE_FROM_ALL vctr_test.cpp)

target_link_libraries (vctr_test PRIVATE vctr vctr_test_utils vctr_test_cases Catch2::Catch2 gcem::gcem)
