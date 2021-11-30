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
into this directory and configure all projects (note `--preset` should specify
suitable for you preset in format
`<cpu_architecture>-<device>-linux-<compiler>-<build_type>` from
[CMakePresets.json](../../CMakePresets.json)):

```
$ cd whitebox
$ cmake --preset x86_64-unknown-linux-clang-release
...
-- Configuring done
-- Generating done
-- Build files have been written to: ${PWD}
```

Now you can build the CMake all target:

```
$ cmake --build --preset x86_64-unknown-linux-clang-release
...
[100%] Linking CXX executable whitebox-base_tests
[100%] Built target whitebox-base_tests
```

Once you have built the CMake tests, run them in parallel with the `ctest`
command:

```
$ ctest --preset x86_64-unknown-linux-clang-release
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
