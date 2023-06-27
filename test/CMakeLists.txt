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
        TestCases/FindingAndManipulatingElements.cpp
        TestCases/Range.cpp
        TestCases/SpanConstructors.cpp
        TestCases/SpanMemberFunctions.cpp
        TestCases/VctrBaseMemberFunctions.cpp
        TestCases/VectorMemberFunctions.cpp
        TestCases/VectorConstructors.cpp
        TestCases/VectorStdPMRCompatibility.cpp

        TestCases/Expressions/Abs.cpp
        TestCases/Expressions/Add.cpp
        TestCases/Expressions/Angle.cpp
        TestCases/Expressions/Clamp.cpp
        TestCases/Expressions/Conjugate.cpp
        TestCases/Expressions/Divide.cpp
        TestCases/Expressions/Exp.cpp
        TestCases/Expressions/Ln.cpp
        TestCases/Expressions/Log2.cpp
        TestCases/Expressions/Log10.cpp
        TestCases/Expressions/Decibels.cpp
        TestCases/Expressions/Multiply.cpp
        TestCases/Expressions/Map.cpp
        TestCases/Expressions/Max.cpp
        TestCases/Expressions/Mean.cpp
        TestCases/Expressions/Min.cpp
        TestCases/Expressions/NormalizeSum.cpp
        TestCases/Expressions/Pow.cpp
        TestCases/Expressions/PowerSpectrum.cpp
        TestCases/Expressions/RealImag.cpp
        TestCases/Expressions/Subtract.cpp
        TestCases/Expressions/Sum.cpp
        TestCases/Expressions/SquareCube.cpp
        TestCases/Expressions/Transformation.cpp
        TestCases/Expressions/Trigonometric.cpp

        TestCases/Generators/Linspace.cpp)

if (VCTR_BUILD_TEST)
    add_subdirectory (TestMain)
endif()

# ================================================================================
add_library (sonible::vctr_test_utils ALIAS vctr_test_utils)
add_library (sonible::vctr_test_cases ALIAS vctr_test_cases)
