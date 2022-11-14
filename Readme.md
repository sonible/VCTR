# VCTR

A powerful C++ 20 wrapper around your favorite standard library containers.

## Getting started

VCTR is a header-only library. The easiest way to use it is to add it as a git submodule to your project:

```bash
# in project root
git submodule add ../vctr.git ext/vctr
```

After doing so, you can simply add it to your project using the `add_subdirectory` command and link against the `sonible::vctr` target:

```cmake
# CMakeLists.txt

cmake_minimum_required (VERSION 3.15)
project (example 0.0.1)

add_subdirectory (ext/vctr)

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
  std::cout << "squaredSum = " << squaredSum << std::endl; // [6, 8, 10, 12]
}
```

Finally, export the CMake project and built it:

```bash
# in project root

# export project
cmake . -B build

# build executable
cmake --build build/

# run executable
build/example
```

In most cases you simply want to `#include <vctr/vctr.h>`.
If you only want to access the forward declarations of the public types you can
`#include <vctr/vctr_forward_declarations.h>` as a lightweight alternative.

It is also possible to access a demo project that showcases some of VCTR's features.
For doing so, export the project using the `-D VCTR_BUILD_DEMO=1` CMake option after cloning:

```bash
# in VCTR project root

# enable exporting of demo project
cmake . -B build -D VCTR_BUILD_DEMO=1

# build demo project
cmake --build build/ --target vctr_demo

# run the demo project
./build/demo/vctr_demo
```

### Dependencies

In order to achieve high performance, VCTR uses two platform specific vector operation libraries, these are:

#### Apple Accelerate

On Apple systems the [Accelerate](https://developer.apple.com/documentation/accelerate) framework is a required dependency.
If you are using VCTR using CMake, the framework will be linked and used automatically; no configuration is necessary.

#### Intel IPP

[Intel IPP](https://www.intel.com/content/www/us/en/developer/tools/oneapi/ipp.html) is an optional dependency that is available on Intel (x64) systems.

To install IPP, please follow the instructions on the website or install it using python: `pip3 install ipp-static`.

Next, use the `VCTR_USE_IPP=1` CMake option to instruct the project to link against IPP.

VCTR will look for the required headers and libraries in common locations. In case you use a non-standard location,
you can pass its location using the `VCTR_IPP_ROOT` command line option when exporting the project:

```bash
# in project root
cmake . -B build -D VCTR_USE_IPP=1

# or in case of non-standard IPP install location
cmake . -B build -D VCTR_USE_IPP=1 -D VCTR_IPP_ROOT=/path/to/ipp
```

### Manual Setup

The documentation up until this point assumes that VCTR is used in a CMake-based project.
If you however use a different setup, you can follow the steps below to use this library:

- Add the top level `include` folder to your header search path.
- (Apple only) Link against the `Accelerate` framework.

In case you want to enable IPP support:

- Add the IPP include location to the header search path, such that `ipp.h` can be found.
- Add the IPP library location to the library search path.
- Link against the required libraries: `ippcore(mt)`, `ipps(mt)` and `ippvm(mt)`.

Note that VCTR will try to use IPP as soon as it can resolve the `ipp.h` header.
In case you want to disable IPP even if the headers are available, define the `VCTR_USE_IPP` preprocessor flag to `0`.

#### JUCE

If you are using VCTR in the context of a [JUCE](https://github.com/juce-framework/JUCE) project that does not yet use CMake,
you can treat `include/vctr` as a JUCE module.

In Projucer, add this directory using the `+` button in the `Modules` tab and select `Add a module from a specific folder`.
After doing so, you can use it as shown in the example at the beginning of this document.


## Contributing and Unit Tests

The project is currently under development. Contributions to the project are highly appreciated, just have a look at the issue section to find open issues. We'll add a contribution guideline to this repository soon.

### Test dependencies

VCTR's unit tests, located in the `test` subfolder, are dependent on [Catch2](https://github.com/catchorg/Catch2) and [gcem](https://github.com/kthohr/gcem).

#### Installation via conan

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

#### Manual installation

In case conan is not used, the dependencies need to be installed manually on the system.
Please consult the project's documentation for details.

The minimal steps include the following:

```bash
# clone gcem from GitHub
git clone https://github.com/kthohr/gcem

# enter cloned folder
cd gcem

# generate Makefiles
cmake . -B build

# switch to build folder
cd build

# install headers on system
sudo make install

# move out of gcem folder
cd ../..

# clone Catch2 from GitHub
git clone https://github.com/catchorg/Catch2.git

# enter cloned folder
cd Catch2

# generate makefiles
cmake -B build -D BUILD_TESTING=OFF

# build library and install catch2 on system
sudo cmake --build build/ --target install
```

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
