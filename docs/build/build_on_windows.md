# Checking out and Building WhiteBox for Windows

There are instructions for other platforms linked from the
[get the code](../get_the_code.md) page.

## System requirements

* A 64-bit Intel/AMD machine with at least 8GB of RAM.  More than 16GB is highly
  recommended.
* At least 10GB of free disk space on an NTFS-formatted hard drive.
* An appropriate version of Visual Studio, as described below.
* Windows 10 or newer.

## Setting up Windows

### Visual Studio

WhiteBox requires Visual Studio 2019 (>=16.11.7) to build.  Visual Studio can
also be used to debug WhiteBox, and VS2019 is preferred for this as it handles
WhiteBox's large debug information much better.

Visual Studio Community Edition should work if its license is appropriate for
you.  You must install the *"Desktop development with C++"* component and the
*"C++ CMake tools for Windows"* sub-components.

You must have the version *10.0.22000 or higher* Windows 10 SDK installed.  This
can be installed separately or by checking the appropriate box in the Visual
Studio Installer.

### Install Git

[Git](https://git-scm.com/) is required for interacting with the WhiteBox source
code repository, which is contained on [GitHub](https://github.com/).
To install Git, consult the [Set Up Git](https://docs.github.com/en/get-started/quickstart/set-up-git)
guide on GitHub.

## Getting the WhiteBox code

* Open `Git Bash` and change to the directory where you want to create the code
  repository:
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

Git will create the repository within a directory named `whitebox`.  Press
`Win` key and type *Developer Command Prompt for VS 2019* to open one.  Then
navigate into this directory and configure all projects (note `--preset` should
specify suitable for you preset in format
`<cpu_architecture>-<cpu_vendor>_pc-windows-<compiler>-<build_type>` from
[CMakePresets.json](../../CMakePresets.json)):

```
$ cd whitebox
$ cmake --preset x86_64-intel_pc-windows-msvc-release
...
-- Configuring done
-- Generating done
-- Build files have been written to: ${PWD}
```

Now you can build the CMake all target:

```
$ cmake --build --preset x86_64-intel_pc-windows-msvc-release
...
[100%] Linking CXX executable whitebox-base_tests
[100%] Built target whitebox-base_tests
```

Once you have built the CMake tests, run them in parallel with the `ctest`
command:

```
$ ctest --preset x86_64-intel_pc-windows-msvc-release
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
