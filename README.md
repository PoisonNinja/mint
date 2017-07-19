# Mint
Mint is the kernel for the [Pepper](https://github.com/PoisonNinja/Pepper.git) operating system built for learning purposes, with an emphasis for readability and simplicity.

Mint currently supports the x86_64 architecture, with future ports for other architectures planned once kernel has reached somewhat of a stable API. As of now, the kernel interface is constantly changing, and it's unrealistic to maintain multiple architectures.

# Features
Mint is currently in the alpha phase, with rapid progress being made towards the 0.0.1 release. The 0.0.1 release will at least have a functioning userspace, with more advanced features coming in later releases.

To track the progress towards the 0.0.1 release, please see Issue [#1](https://github.com/PoisonNinja/mint/issues/1).

# Toolchain
The toolchain build process is handled by Pepper as the userspace and kernel share the same toolchain, just with different flags. Although you *can* build the toolchain manually, it is not recommended as it is error-prone. Thus, please visit the Pepper repository for build instructions.

Mint uses the LLVM toolchain for both compiling and linking, removing the need for GCC and ld. Originally, Mint required binutils because we needed ld to be installed, but ld.lld has replaced ld.

The toolchain built by Pepper includes the llvm-* suite of tools that replace the other binutils programs, including nm and objdump. Those share the same flags as the GNU versions, and they can be called by prefixing llvm- to their name. For example, nm becomes llvm-nm, and objdump becomes llvm-objdump.

# Building
Mint uses the [CMake](https://cmake.org/) build generator tool. Thus, you can actually use your preferred build system, such as Make or Ninja, to build Mint.

In tree builds are not supported to prevent cluttering the tree up with object files, and attempting to do so will cause an error.

To start, create a folder named anything you like in the Mint root directory, although I personally call it `build`. Enter the directory, and run `cmake ../`. The default options assume that the architecture is `x86_64`, and that you want to use Make as your build system. For customization options, see the table below. Pass them in when running `cmake` as `-DOPTION=VALUE`.

|Flag                |Description               | Values   |
|--------------------|--------------------------|----------|
|ARCH                |Architecture to build for | `x86_64` |
|CMAKE_TOOLCHAIN_FILE|Path to toolchain spec    | `<path>` |
|TOOLCHAIN_PREFIX    |Path to toolchain         | `<path>` |
|PLATFORM_FILE       |Path to platform spec     | `<path>` |
|CMAKE_C_FLAGS       |Additional CFLAGS         | `<text>` |
|SYSROOT             |Path to Pepper sysroot    | `<path>` |

If you want to use a different build system, such as Ninja, run `cmake -G <generator> ..`. Replace generator with the name of the build system. The full list of generators can be found by running `cmake --help`. I personally build Mint using Ninja, but other build systems should work just fine. The Makefile in Pepper calls cmake instead of directly calling the build system, so switching build systems should not cause any problems with the Pepper build system.

After generating the build files, simple run `<your build system>` in the directory you created, or run `make` in the Pepper directory. The default target will build Mint. To install it into the sysroot configured earlier, run `<your build system> install`.

# License
Mint is licensed under the BSD-3-Clause license.
