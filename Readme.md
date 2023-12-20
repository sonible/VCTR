# VCTR

A powerful C++ 20 wrapper around your favorite standard library containers.
This library is currently under active development, expect a 95% stable API in the current state.

## What it is all about?
VCTR is a library that makes working with vectors of any kind extremely expressive and easy while creating extremely
well optimized code at the same time. We at sonible build real-time audio processing applications where we care a lot
about code that is guaranteed to be heap allocation free, so a major part of the functionality was designed with that
goal in mind. In order to be compatible to existing code that uses containers from the C++ standard library, we try to
wrap classes from the standard library wherever possible. The VCTR project has two main building blocks:

### Containers and views
VCTR has two container class templates: `vctr::Vector`, a `std::vector` backed resizable heap allocated container and 
`vctr::Array`, a `std::array` backed fixed size stack container.
Besides that, it also has a class template that can act as a view to externally owned data which is `vctr::Span`, a 
`std::span` backed class which however derives from the same base class as `vctr::Vector` and `vctr::Array`. That
common base class makes it easy to work on both, data contained in VCTR containers and data from external non-VCTR APIs.

### (Math) Expressions
Most of the time when working with vectors, we want to call a certain function on all elements of a vector, perform
element wise computations to combine the values of multiple source vectors, reduce the content of a vector or often a
combination from all of these tasks. Expressions aim to offer a straightforward syntax for chained modifications on
vectors while avoiding the creation of intermediate temporary vectors at all cost.

Let's look at a first example:
```c++
// vctr::Array is a compile-time static sized stack container based on std::array.
vctr::Array a { 1, 2, 3, 4 };

// vctr::Vector is a dynamic sized heap container based on std::vector.
vctr::Vector b { 5, 6, 7, 8 };

// The returned value is the unevaluated expression, basically a set of instructions how to compute
// that chain of operations. In most cases, lines like these boil down to zero CPU instructions
auto eRaisedToSum = vctr::exp << (a + b);

// Let's create a vctr::Span that acts as a view to some plain C array
float cArray[4];
vctr::Span c (cArray);

// Now we can assign the expression to the span. This will trigger the actual computation and will
// write the result directly into the viewed memory
c = eRaisedToSum;

assert (cArray[0] == std::exp (float (a[0] + b[0])));

// Of course, we can also assign expressions to destination containers right away as one-liners:
vctr::Array d = vctr::exp << (a + b);
```

We can see that expressions are built up from chaining individual operations with the operator `<<`. It can also be seen
that VCTR makes use of class argument template deduction wherever posible. `a` is of type `vctr::Array<int, 4>`,
`b` is of type `vctr::Vector<int>`, `c` is of type `vctr::Span<float, 4>` and `d` is of type `vctr::Array<float, 4>`. 
Note that the source operands are integer, but the `vctr::exp` expression transforms `int` inputs into `float` outputs.
Also, if one operand has a compile-time static size like `a` in this case, the code assumes an expression result based
on that operand to have the same size. If the expression contains run-time dynamic sized operands, debug builds will
insert a run-time assertion that warns the developer if the size does not match.

Expressions don't necessarily need to be mathematical expressions. We can use them to apply all kinds of transformations
on vectors. For an example, we can convert a numerical vector into a vector of strings:
```c++
const vctr::Vector numbers = { -1.0, -42.0, 1.0, 42.0 };
const vctr::Vector strings = vctr::transformedBy ([] (auto n) { return std::to_string (n); }) << numbers;
```

`strings` will now be a `vctr::Vector<std::string>`containing string representations of all elements from `numbers`.

For an overview of all available expressions, there is an [Expressions section in the documentation](https://sonible.github.io/VCTR/group__Expressions.html).

### Member functions
Besides the expressions, VCTR also offers quite a few member functions on the container and view classes. First of all,
every member function from the wrapped standard library classes is forwarded. We decided to stick to the standards
library `snake_case` style for them, although the VCTR project uses `camelCase` for member functions everywhere else.
This makes it easy to identify such forwarded member functions of `std::vector`, `std::array` and `std::span`. On top 
of that, VCTR adds a lot of convenient functions that make tasks like finding and rearranging elements, accessing
sub-spans of a vector and in case of `vctr::Vector` adding and removing elements:
```c++
vctr::Vector numbers { 1, 2, 3, 4, 5, 6, 7, 8 };

// A sub span starting at index 3, containing 5 elements
auto s = numbers.subSpan<3, 5>();

// Reverses the elements in the sub span, numbers becomes [ 1, 2, 3, 8, 7, 6, 5, 4 ]
s.reverse();

// Subtracts 2 from all elements in the sub span, numbers becomes [ 1, 2, 3, 6, 5, 4, 3, 2 ]
s -= 2;

// Will be 3
auto i = numbers.indexOfMaxElement();

// Only available for vctr::Vector, numbers becomes [ 1, 2, 6, 5, 4, 2 ]
numbers.eraseAllOccurrencesOf (3);

// A compact alternative to a classic for loop, numbers becomes [ 1, 0, 0, 1, 0, 0 ]
numbers.forEach ([] (auto& n) -> void { n %= 2; });
```

For an overview of all available member functions, have a look at the [Core Types section in the documentation](https://sonible.github.io/VCTR/group__Core.html).

### Passing VCTR types as function arguments
If you want to write functions that take VCTR types as arguments, you have multiple options. If you care about
maximum performance, it's recommended to write a function template and pass it as concept constrained template:
```c++
template <vctr::is::anyVctr T>
void doSomethingFancy (T&& vec)
{
    //... do something fancy with vec
}
```

This approach allows the compiler to fully inline the function call. Furthermore, most VCTR types have additional
optional template arguments that cary information about e.g. the alignment of memory etc. With the template approach,
these informations are still visible within the function. If you want to constrain the value type that can be passed
to the function you can also use a more specialised constrain:
```c++
template <vctr::is::anyVctrWithValueType<int64_t> T>
void doSomethingFancyWithAnInt64Vec (T&& vec)
{
    //... do something fancy with vec
}
```

You can also allow unevaluated expressions to be passed to a function, like e.g.
```c++
template <vctr::is::anyExpressionWithValueType<int64_t> T>
void doSomethingFancyWithAnInt64Expression (T&& expression)
{
    //... do something fancy with expression
}

template <vctr::is::anyVctrOrExpression T>
void doSomethingFancy (T&& vecOrExpression)
{
    //... do something fancy with vecOrExpression
}
```

Accordingly, when you are returning VCTR instances created from within a function you should consider an `auto` return
type, which has the same advantages of keeping the entire metadata kept in a certain instance. In general, you should
stick to the almost always auto rule, e.g. these two lines will possibly be not the same:
```c++
vctr::Vector<int> vec (10);
auto s1 = vec.subSpan<2, 5>();            // Will have compiletime metadata regarding the size and memory alignment
vctr::Span<int> s2 = vec.subSpan<2, 5>(); // Will compute size and memory alignment at runtime
```

Still, there might be cases where function templates are no suitable choice. If you want to accept all kinds of VCTR
types as arguments to those functions, use `vctr::Span` as type. For read-only containers, qualify the value type 
argument of the span as `const`, for mutable container use the non-const equivalent:
```c++
void readAndWrite (const vctr::Span<const float>& src, const vctr::Span<float>& dst);
```

`vctr::Span` has implicit constructors that will make a wide range of arguments silently convert to these types, which
makes the above approach suitable for cases where non-vctr source and destination containers are involved.

Last but not least, there might be cases where the exact container type is known. In that case, it's of course also
a solid approach to just pass the exact types by reference.

## Integrating VCTR into your project

VCTR is a CMake based header-only C++ 20 library. The recommended way to use it is to add it as a git submodule in your 
project:

```bash
# in project root
git submodule add git@github.com:sonible/VCTR.git third_party/vctr
```

### Integration into CMake based projects
If your project uses CMake, you can simply add it to your CMake project using the `add_subdirectory` command and link 
against the `sonible::vctr` target:

```cmake
# CMakeLists.txt

cmake_minimum_required (VERSION 3.15)
project (example 0.0.1)

add_subdirectory (third_party/vctr)

add_executable (example main.cpp)
target_link_libraries (example PRIVATE sonible::vctr)
```

To use it in your project, include the `vcrt/vctr.h` header:

```c++
// main.cpp

#include <iostream>
#include <vctr/vctr.h>

int main()
{
  // vctr::Array is a compile-time static sized stack container based on std::array.
  vctr::Array a { 1.0, 2.0, 3.0, 4.0 };

  // vctr::Vector is a dynamic sized heap container based on std::vector.
  vctr::Vector b { 5.0, 6.0, 7.0, 8.0 };
  
  // vctr::Span is a view to vector data owned by other containers based on std::span
  vctr::Span s = a;

  // Use overloaded << operator to perform mathematical functions, general purpose math operators are also overloaded.
  auto squaredSum = vctr::square << (a + b);

  // We can pass it to an output stream
  std::cout << "squaredSum = " << squaredSum << std::endl; // [36, 64, 100, 144]
}
```

You should be ready to use VCTR now!

In most cases you simply want to `#include <vctr/vctr.h>`. If you only want to access the forward declarations of the 
public types you can `#include <vctr/vctr_forward_declarations.h>` as a lightweight alternative.

### Dependencies and Requirements

VCTR is using cutting-edge C++ library features and needs a recent compiler to work properly.
It is currently tested with the following compilers:
- Clang 15+
- GCC 12+ (Linux only)
- Visual Studio 2022+
- XCode 14+

In order to achieve high performance, VCTR uses two platform specific vector operation libraries, these are:

#### Apple Accelerate

On Apple systems the [Accelerate](https://developer.apple.com/documentation/accelerate) framework is a required dependency.
If you are using VCTR using CMake, the framework will be linked and used automatically; no configuration is necessary.

#### Intel IPP

[Intel IPP](https://www.intel.com/content/www/us/en/developer/tools/oneapi/ipp.html) is an optional dependency that is 
available on Intel (x64) systems. All VCTR features will work without it, but you might see an increase in performance.

To install IPP, please follow the instructions on the intel website or install it using python: `pip3 install ipp-static`.

If you want VCTR to conveniently take care of linking against an existing IPP installation, use the 
`VCTR_AUTOLINK_IPP=1` CMake option to instruct VCTR to find IPP and link against it.

VCTR will look for the required headers and libraries in common locations. In case you use a non-standard location,
you can pass its location using the `VCTR_IPP_ROOT` command line option when exporting the project:

```bash
# in project root
cmake . -B build -D VCTR_AUTOLINK_IPP=1

# or in case of non-standard IPP install location
cmake . -B build -D VCTR_AUTOLINK_IPP=1 -D VCTR_IPP_ROOT=/path/to/ipp
```

There might be cases where you want to integrate VCTR into a project that already links against IPP itself. In that
case you have to do nothing special. VCTR will try to use IPP as soon as it can resolve the `ipp.h` header.
In case you want to disable usage of  IPP even if the headers are available, define the `VCTR_USE_IPP` preprocessor 
flag to `0`.

#### GCE-Math
GCE-Math (Generalized Constant Expression Math) is a templated C++ library enabling compile-time computation of 
mathematical functions. The header only [gcem library](https://github.com/kthohr/gcem) is an optional dependency that
allows us to use a lof of VCTRs math functions as `constexpr` to perform computations in compile time evaluated 
contexts.

VCTR will try to use GCE-Math as soon as it can resolve the `gcem.hpp` header. The easiest way to achieve that is to
integrate GCE-Math via CMake and link against the `gcem` CMake target. In case you want to disable usage of GCE-Math 
even if the headers are available, define the `VCTR_USE_GCEM` preprocessor flag to `0`.

Note that the minimum required version of GCE-Math is 1.16.0.

### Manual Setup

The documentation up until this point assumes that VCTR is used in a CMake-based project.
If you however use a different setup, you can follow the steps below to use this library:

- Add the top level `include` folder to your header search path.
- (Apple only) Link against the `Accelerate` framework.

In case you want to enable IPP support:

- Add the IPP include location to the header search path, such that `ipp.h` can be found.
- Add the IPP library location to the library search path.
- Link against the required libraries: `ippcore(mt)`, `ipps(mt)` and `ippvm(mt)`.

In case you want to enable GCE-Math:
- Add an installation of GCE-Math 1.16.0 or higher to your header search path

## Building unit tests and demo project

It is also possible to access a demo project that showcases some of VCTR's features.
To build the demo project, export the project using the `-D VCTR_BUILD_DEMO=1` CMake option after cloning:

```bash
# in VCTR project root

# enable exporting of demo project
cmake . -B build -D VCTR_BUILD_DEMO=1

# build demo project
cmake --build build/ --target vctr_demo

# run the demo project
./build/demo/vctr_demo
```

VCTR's unit tests, located in the `test` subfolder, are dependent on [Catch2](https://github.com/catchorg/Catch2) and
GCE-Math. 

The preferred method of installing the dependencies is using the C++ package manager [conan](https://github.com/conan-io/conan).
If `conan` is available on your system, export the project using the `VCTR_USE_CONAN=1` and `VCTR_BUILD_TEST=1` options:

```bash
# conan is available via chocolatey, homebrew or pip3

# windows
choco install conan

# macOS
brew install conan

# linux
pip3 install conan

# in VCTR project root

# export project and tests
cmake . -B build -D VCTR_USE_CONAN=1 -D VCTR_BUILD_TEST=1

# build tests
cmake --build build/ --target vctr_test

# run tests
./build/test/vctr_test
```

In case you want to integrate the VCTR unit test cases into your own catch2 based unit tests, you can define 
`VCTR_EXPOSE_UNIT_TEST_CASES_TARGET` to 1 and link against the `sonible::vctr_test_targets` target which contains all 
test cases but not the test main.

## Contributing

The project is currently under development. Contributions to the project are highly appreciated, just have a look at 
the issue section to find open issues. We'll add a contribution guideline to this repository soon.

### Project structure

(This section is still under development, more infos will follow)

- By convention, all `snake_case` headers are intended to be included by the user, `CamelCase` headers are
  implementation details and cannot be included on their own.

- Headers included via `#include <vctr_utils/...>` supply additional utility functions that are not strictly relevant
  to the core functionality of the VCTR project.
  They are used in the test, benchmark, and demo projects and might have additional third party dependencies.

- Headers included via `#include <vctr_test_utils/...>` supply additional utility functions that are useful especially
  for Catch2-based tests. They depend on having Catch2 greater version 3.0.0 available. If you are writing Catch2 based
  unit tests, you might be especially interested in the matchers that can be accessed via
  `#include <vctr_test_utils/vctr_catch_matchers.h>`.

### Naming conventions

(This section is still under development, more infos will follow)

This section explains some commonly used names and concepts that are used throughout the project.

### extent

The extent definition is basically taken from `std::span`. In all situations where the size of a container,
a view, or an expression is known at compile time, the extent will equal the size.
In cases where the size is only known at runtime, the extent will equal `std::dynamic_extent` which in turn is defined to
`std::numeric_limits<size_t>::max()` on all platforms we know. A `Span` might have both, a dynamic and a non-dynamic extent.
A `Vector` will always have a dynamic extent. An `Array` will always have a non-dynamic extent.
Expressions will inherit the extent of their source(s).


## License information

VCTR is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License version 3 (LGPLv3) only,
as published by the Free Software Foundation.

This basically implies, in legally non-binding terms:

- You may _use_ VCTR in projects of any kind, regardless of their purpose, context or commercial status,
  as long as the use of VCTR and its copyright is documented in the project's disclaimer.
- You may _use_ VCTR also with code whose licensing terms are incompatible with that of the
  GNU General Public License (GPL), including non-free, proprietary or so-called _closed source_ code.
  VCTR is a header-only library, so all regulations of the LGPLv3 license regarding static or dynamic
  library linkage do not apply here.
- Following from the previous, merely _using_ VCTR as is does not require any changes to the licensing terms
  of your code. In particular, proprietary code using VCTR may remain proprietary and so-called _closed source_
  even if your code incorporates VCTR at compile-time due to its nature of a header-only library.
- You may _modify, adapt_ or _extend_ VCTR according to your requirements, but if you do so, you must make
  the source code of your modifications available as soon as you distribute your modifications in binary form,
  for instance, in a project release that involves your modified version of VCTR.
- Your modifications will underlie the same licensing terms of the LGPLv3 as VCTR. You must not remove
  any license information from the file headers or from other project files such as LICENSE.
- You are not obliged to contribute your extensions directly to sonible, for instance, through patches or
  pull requests, but you are cordially invited to do so.

Please find the full licensing terms and conditions in the LICENSE file.
