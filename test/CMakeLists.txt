cmake_minimum_required (VERSION 3.15)
# ================================================================================
add_library (vctr_test_utils INTERFACE)
target_include_directories (vctr_test_utils INTERFACE include)

# ================================================================================
add_library (vctr_test_cases INTERFACE)

target_sources (vctr_test_cases INTERFACE
        TestCases/InternalFunctions.cpp

        TestCases/ArrayConstructors.cpp
        TestCases/ArrayMemberFunctions.cpp
        TestCases/ConversionOperators.cpp
        TestCases/ElementAccessFunctions.cpp
        TestCases/EqualsMatcher.cpp
        TestCases/FindingAndManipulatingElements.cpp
        TestCases/Range.cpp
        TestCases/ScalarCompareOp.cpp
        TestCases/SpanConstructors.cpp
        TestCases/SpanMemberFunctions.cpp
        TestCases/VctrBaseMemberFunctions.cpp
        TestCases/VectorMemberFunctions.cpp
        TestCases/VectorConstructors.cpp
        TestCases/VectorStdPMRCompatibility.cpp
        TestCases/BitwiseMaskOperations.cpp

        TestCases/Containers/SplitComplex.cpp

        TestCases/Expressions/Abs.cpp
        TestCases/Expressions/Add.cpp
        TestCases/Expressions/Angle.cpp
        TestCases/Expressions/BinaryCaseDistinction.cpp
        TestCases/Expressions/Clamp.cpp
        TestCases/Expressions/Conjugate.cpp
        TestCases/Expressions/Divide.cpp
        TestCases/Expressions/ElementwiseMax.cpp
        TestCases/Expressions/ElementwiseMin.cpp
        TestCases/Expressions/Exp.cpp
        TestCases/Expressions/Forward.cpp
        TestCases/Expressions/Ln.cpp
        TestCases/Expressions/Log2.cpp
        TestCases/Expressions/Log10.cpp
        TestCases/Expressions/Decibels.cpp
        TestCases/Expressions/Multiply.cpp
        TestCases/Expressions/Map.cpp
        TestCases/Expressions/Max.cpp
        TestCases/Expressions/Mean.cpp
        TestCases/Expressions/Min.cpp
        TestCases/Expressions/MultiplyAccumulate.cpp
        TestCases/Expressions/NormalizeSum.cpp
        TestCases/Expressions/Pow.cpp
        TestCases/Expressions/PowerSpectrum.cpp
        TestCases/Expressions/Product.cpp
        TestCases/Expressions/RealImag.cpp
        TestCases/Expressions/SIMDFilter.cpp
        TestCases/Expressions/Subtract.cpp
        TestCases/Expressions/Sum.cpp
        TestCases/Expressions/SquareCube.cpp
        TestCases/Expressions/Transformation.cpp
        TestCases/Expressions/Trigonometric.cpp
        TestCases/Expressions/FastExp.cpp

        TestCases/Generators/Linspace.cpp

        TestCases/Utils/CompileTimeRandomArray.cpp)

# The random test data is generated at compile time. Without VCTR_RANDOM_SEED, every translation unit would derive its
# own seed from the time it is compiled at, so a single seed could not be printed and reused. Therefore, one seed is
# chosen per configure run and shared by all test cases. vctr_test prints it at startup, pass it back via
# -D VCTR_TEST_RANDOM_SEED=<seed> to reproduce the test data of that build.
set (VCTR_TEST_RANDOM_SEED "" CACHE STRING "Seed for the compile time random test data. Empty: new seed each configure run")

if (VCTR_TEST_RANDOM_SEED STREQUAL "")
    string (TIMESTAMP VCTR_TEST_EFFECTIVE_RANDOM_SEED "%s" UTC)
else()
    if (NOT VCTR_TEST_RANDOM_SEED MATCHES "^[1-9][0-9]*$" OR VCTR_TEST_RANDOM_SEED GREATER 4294967295)
        message (FATAL_ERROR "VCTR_TEST_RANDOM_SEED must be a positive 32 bit integer, got '${VCTR_TEST_RANDOM_SEED}'")
    endif()

    set (VCTR_TEST_EFFECTIVE_RANDOM_SEED ${VCTR_TEST_RANDOM_SEED})
endif()

message (STATUS "VCTR: Random seed for the unit test data: ${VCTR_TEST_EFFECTIVE_RANDOM_SEED}")
target_compile_definitions (vctr_test_cases INTERFACE VCTR_RANDOM_SEED=${VCTR_TEST_EFFECTIVE_RANDOM_SEED}u)

if (VCTR_BUILD_TEST)
    add_subdirectory (TestMain)
endif()

# ================================================================================
add_library (sonible::vctr_test_utils ALIAS vctr_test_utils)
add_library (sonible::vctr_test_cases ALIAS vctr_test_cases)
