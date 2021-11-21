# Checking out and Building WhiteBox for Linux distros

There are instructions for other platforms linked from the
[get the code](../get_the_code.md) page.

## System requirements

* A 64-bit Intel/AMD machine with at least 8GB of RAM.  More than 16GB is highly
  recommended.
* At least 10GB of free disk space.
* You must have [Git](https://git-scm.com/) and [CMake 3.10+](https://cmake.org/)
  installed already (and `cmake` must point to a CMake binary).

## Getting the WhiteBox code

* Change to the directory where you want to create the code repository:
```
$ cd ~
$ mkdir source && cd source
```

* Clone code repository with submodules:
```
$ git clone --recurse-submodules https://github.com/The-White-Box/whitebox.git
Cloning into 'whitebox'...
remote: Enumerating objects: 149, done.
...
Resolving deltas: 100% (1083/1083), done.
```

## Setting up the build

WhiteBox uses [Ninja](https://ninja-build.org) as its main build tool along with
a tool called [CMake](https://cmake.org/) to generate `.ninja` files via
CMake's Ninja backend.

WhiteBox uses SDL library, which requires additional packages.  Please, follow
[SDL Linux README](../../deps/sdl/docs/README-linux.md) to add the packages.

Git will create the repository within a directory named `whitebox`.  Navigate
into this directory and run all tests:

```
$ cd whitebox
$ mkdir build && cd build
$ cmake -DADD_FATAL_EXAMPLE=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCHANGE_G3LOG_DEBUG_TO_DBUG=ON -Dgtest_hide_internal_symbols=ON \
  -DINSTALL_GTEST=OFF -DMI_BUILD_OBJECT=OFF -DMI_BUILD_STATIC=ON \
  -DMI_BUILD_TESTS=ON -DMI_SECURE=ON -DWB_BUILD_TESTS=ON \
  -DWB_CLANG_LANGUAGE_VERSION="c++20" -DWB_GCC_LANGUAGE_VERSION="c++20" \
  -DWB_GCC_ENABLE_WARNING_WUNDEF=OFF ..
...
-- Configuring done
-- Generating done
-- Build files have been written to: ${PWD}
```

Now you can build the CMake target tests:

```
$ cmake --build . --target all
...
[100%] Linking CXX executable whitebox-base_tests
[100%] Built target whitebox-base_tests
```

Once you have built the CMake tests, run them in parallel with the `ctest`
command:

```
$ ctest
Test project ${PWD}
      Start  1: whitebox-base_tests
...
100% tests passed, 0 tests failed out of 890
```

### Faster builds

* Reduce file system overhead by excluding build directories from antivirus
  and indexing software.
* Store the build tree on a fast disk (preferably SSD).
* The more cores the better (16+ is not excessive) and lots of RAM is needed
  (32 GB is not excessive).
