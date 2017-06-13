# Mint
Mint is a architecture-portable kernel for the [Pepper](https://github.com/PoisonNinja/Pepper.git) operating system. Originally started as a rewrite of the Strawberry kernel, Mint features improvements in many areas from the build system to the console subsystem, and incorporates many of the lessons learned from writing Strawberry.

# Toolchain
The toolchain build process is handled by Pepper. Although you *can* build it manually, it is not recommended. Thus, please visit the Pepper repository for build instructions.

# Building
Mint uses the [CMake](https://cmake.org/) build generator tool. Thus, you can actually use your preferred build system, such as Make or Ninja, to build Mint.

In tree builds are not supported to prevent cluttering the tree up with object files, and attempting to do so will cause an error.

To start, create a folder named anything you like, although I personally call it `build`. Enter the directory, and run `cmake ../`. The default options assume that you followed the exact instructions from Pepper, your directory layout matches the default, and the architecture is `x86_64`. For customization options, see the table below. Pass them in when running `cmake` as `-DOPTION=VALUE`.

|Flag                |Description               | Values   |
|--------------------|--------------------------|----------|
|ARCH                |Architecture to build for | `x86_64` |
|CMAKE_TOOLCHAIN_FILE|Path to toolchain spec    | `<path>` |
|TOOLCHAIN_PREFIX    |Path to toolchain         | `<path>` |
|PLATFORM_FILE       |Path to platform spec     | `<path>` |
|CMAKE_C_FLAGS       |Additional CFLAGS         | `<text>` |
|SYSROOT             |Path to Pepper sysroot    | `<path>` |

If you want to use a different build system, such as Ninja, run `cmake -G <generator> ..`. Replace generator with the name of the build system. The full list of generators can be found by running `cmake --help`. I test Mint using Ninja. Other build systems are not guaranteed to work. The rest of the instructions will assume that you use Ninja, although it should be incredibly easy to use a different system. In fact, Pepper calls cmake instead of directly calling the build generator, so switching build generators should not cause any problems with Pepper.

After generating the build files, simple run `<your build system>`. The default target will build Mint. To install it into the sysroot configured earlier, run `<your build system> install`.

# License
Mint is licensed under the GPLv2 license.

Portions of the codebase (notably the linked list and the clocksource_calculate function) are from Linux. The appropriate license headers have been included. In the future, as the code is phased out, those notices will be removed as they no longer apply.
