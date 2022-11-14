cmake_minimum_required (VERSION 3.15)

project (VCTR_DEMO)

# ================================================================================
add_executable (vctr_demo EXCLUDE_FROM_ALL demo.cpp)
target_link_libraries (vctr_demo PRIVATE vctr)
