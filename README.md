# DcpClient

DcpClient is a Qt-based C++ library for the Device Communication Protocol
(DCP), primarily used at the GREGOR solar telescope on Tenerife.

This package consists of

- `libDcpClient`: The DCP C++ library
- `dcpterm`: A GUI for communicating with DCP devices
- `dcphub`: A basic implementation of a DCP server

as well as various other tools and examples.


## Build requirements

- Qt 4.7 or Qt 5
- CMake 2.8.12 or newer


## How to install

Extract the source archive (or checkout the Git repository). Change into the
`dcpclient` directory and create a `build` subdirectory. Change into the
`build` directory and run `cmake`, `make` and `make install`.

    cd dcpclient
    mkdir build

    cd build
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/opt/dcpclient \
        -DBUILD_TOOLS=ON

    make
    make install

For the `make install` command you need to have write access to the directory
specified by the `CMAKE_INSTALL_PREFIX` option. You can, for example, use
`sudo make install`, or create the destination directory manually and make
sure that your (non-root) user has access to it (recommended).


## Build options

There are a number of CMake build options available. These options can be
set using `-DOPTION_NAME=VALUE` when running `cmake`, or by using the CMake
GUI `cmake-gui` (Qt-based) or `ccmake` (ncurses-based).

- `CMAKE_BUILD_TYPE`:
  CMake build type (e.g. `"Release"` or `"Debug"`;
  default: `""`)
- `CMAKE_INSTALL_PREFIX`:
  Installation base directory
  (Linux default: `"/usr/local"`)
- `BUILD_TOOLS`:
  Build utility programs `dcpterm`, `dcphub`, and `dcpsend`
  (default: `OFF`)
- `BUILD_EXAMPLES`:
  Build example programs `dcpdump`, `dcplisten`, `dcptime`
  (default: `OFF`)
- `BUILD_USE_QT4`: Use Qt4 even if Qt5 is installed
  (default: `OFF`)
- `BUILD_STATIC_LIBRARY`:
  Build a static `DcpClient` library
  (default: `OFF`)
- `INSTALL_STATIC_LIBRARY`:
  Install static library if static build was enables
  (default: `OFF`)
- `BUILD_DOCUMENTATION`:
  Build Doxygen documentation
  (default: `OFF`)
- `INSTALL_EXAMPLE_SOURCES`:
  Install example source code
  (default: `OFF`)


## Example configurations

Below are some examples for configuring the build with the `cmake` command.
These examples assume that you already changed to the `dcpclient/build`
directory as described above. For compiling and installing you still need
to run `make` and `make install` after using `cmake`. If you are trying out
different builds in the same build directory, it might be necessary to run
`make clean` first, and to delete the `CMakeCache.txt` before re-executing
the `cmake` command.


### Only build and install the library

The following will only build the DcpClient library, without any utility
programs. The header files and the shared library will be installed using
the default installation prefix (e.g. `/usr/local` on Linux).

    cmake .. -DCMAKE_BUILD_TYPE=Release


### Build tools and shared library, install to `/opt/dcpclient`

This is a typical way of building the shared DcpClient library together
with `dcpterm`, `dcphub` and `dcpsend`, and install it to `/opt/dcpclient`.

    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/opt/dcpclient \
        -DBUILD_TOOLS=ON

On Unix-like systems RPATH is used by default. If you want to make the
installation directory relocatable, you can use the `CMAKE_INSTALL_RPATH`
option with `$ORIGIN` on Linux or `@loader_path` on MacOS. To make the
above example relocatable on Linux you would write:

    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/opt/dcpclient \
        -DCMAKE_INSTALL_RPATH='$ORIGIN/../lib' \
        -DBUILD_TOOLS=ON

Note that the single quotation marks in `'$ORIGIN/../lib'` are important to
prevent the shell from interpreting `$ORIGIN` as a variable.


### Statically build tools and examples only, install to build directory

The following example builds all executables (tools and examples) and links
statically against the DcpClient library. In this case the programs do not
depend on a shared DcpClient library, which can be useful if the library is
not needed by any other application.

    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="${PWD}/install" \
        -DBUILD_TOOLS=ON \
        -DBUILD_EXAMPLES=ON \
        -DBUILD_STATIC_LIBRARY=ON

With the command above the executables are installed to the `build/install`
directory when running `make install`.

Note: If you want to copy the statically linked executables to a different
directory, you should use the binaries from the `build/install/bin`
directory, not the binaries that are created in the root of the `build`
directory.
