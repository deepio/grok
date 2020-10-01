# How to Build and Install Grok binaries

Grok uses [cmake](www.cmake.org) to configure builds across multiple platforms.

To configure a build using the defaults:

```
   $ mkdir /PATH/TO/BUILD
   $ cd /PATH/TO/BUILD
   $ cmake /PATH/TO/SOURCE
```

On headless systems, `ccmake` (an ncurses application) may be used to configure the build.
If you are running Windows, OSX or X-Windows, then the `cmake` gui may be used.


## UNIX/LINUX/OSX

### SHARED/STATIC

The `BUILD_SHARED_LIBS` `cmake` flag determines if the `grk_compress` and `grk_decompress` binaries are
linked to dynamic or static builds of the codec library `libgrokj2k`. If both `BUILD_SHARED_LIBS`
and `BUILD_STATIC_LIBS` `cmake` flags are set for a non-windows build, then both dynamic and static builds are generated,
and the static version is installed.


### DEBUG/RELEASE

Default build type is `Release`. For a `Debug` build, run:

`$ cmake -DCMAKE_BUILD_TYPE=Debug /PATH/TO/SOURCE`

### Build

`$ make -j8`

for a machine with 8 logical cores.

Binaries are located in the `bin` directory.

### INSTALL

Root users may run:

`$ make install`

those with sudo powers may run:

`$ sudo make install`

and everyone else can run:

`$ DESTDIR=$HOME/local make install`

Note: On Linux, after a shared library build, run

`$ sudo ldconfig`

to update the shared library cache.

### DOCUMENTATION

To build the Doxygen documentation (Doxygen needs to be found on the system):

`$ make doc`

A `HTML` directory is generated in the `doc` directory

### CMAKE FLAGS

Important cmake flags:

* To specify the install path: use `-DCMAKE_INSTALL_PREFIX=/path`, or use `DESTDIR` env variable (see above)
* To build the shared libraries and link the executables against it:

 `-DBUILD_SHARED_LIBS:bool=on` (default: `ON`)

  Note: when using this option, static libraries are not built and executables are dynamically linked.
* To build the CODEC executables: `-DBUILD_CODEC:bool=on` (default: `ON`)
* To build the documentation: `-DBUILD_DOC:bool=on` (default: `OFF`)
* To enable testing (and automatic result upload to http://my.cdash.org/index.php?project=grok)

      $  cmake . -DBUILD_TESTING:BOOL=ON -DGRK_DATA_ROOT:PATH='path/to/the/data/directory'
      $  make
      $  make Experimental

Note : JPEG2000 test files can be cloned [here](https://github.com/GrokImageCompression/grok-test-data.git)


If the `-DGRK_DATA_ROOT:PATH` option is omitted, test files will be automatically searched for in

 `${CMAKE_SOURCE_DIR}/../grok-test-data`


### CPU Specific Optimizations

For Intel and AMD processors, Grok implements optimizations using the `SSE4.1`
instruction set (example: 9x7 inverse MCT transform) and the `AVX2`
instruction set (example: 5x3 inverse discrete wavelet transform).
Currently, those optimizations are only available if Grok is built to
use those instruction sets, and the resulting binary will only run
on compatible CPUs.

With `gcc`/`clang`, it is possible to enable those instruction sets
with the following commands:

```
cmake -DCMAKE_CXX_FLAGS="-O3 -msse4.1 -DNDEBUG" ..
```

```
cmake -DCMAKE_CXX_FLAGS="-O3 -mavx2 -DNDEBUG" ..
```

(`AVX2` implies `SSE4.1`)

Or if the binary will run on the machine where it has
been compiled :

```
cmake -DCMAKE_CXX_FLAGS="-O3 -march=native -DNDEBUG" ..
```


## OSX

OSX builds are configured similar to Unix builds.

The xcode project files can be generated using:

`$ cmake -G Xcode ....`


## WINDOWS


### SHARED/STATIC

The `BUILD_SHARED_LIBS` `cmake` flag determines if the `grk_compress` and `grk_decompress` binaries are linked to dynamic or static builds of the codec library `libgrokj2k`, and also if a static or dynamic version of `libgrokj2k` is built on the system.


### Compile

cmake can generate project files for various IDEs: Visual Studio, NMake, etc.

Type `cmake --help` for available generators on your platform.

### Third Party Libraries

Make sure to build the third party libs (`libpng`, `zlib` etc.) :

  `-DBUILD_THIRDPARTY:BOOL=ON`

 #### JPEG Support

To open JPEG files, a `libjpeg` compatible library (-dev version) must be installed.
Recommended : `libjpeg-turbo`

https://github.com/libjpeg-turbo/libjpeg-turbo .

On debian systems, the `libjpeg-turbo8-dev` package will provide a development
version of the library.

##### Grok dynamic build with JPEG support (Windows)

`libjpeg-turbo` must be built with the `WITH_CRT_DLL` flag on, to ensure that the dynamic version of the C runtime libraries is used. Also, if Grok is linking with dynamic build of `libjpeg-turbo`, (cmake flag `JPEG_LIBRARY` is set to `LIBJPEG_INSTALL_DIRECTORY/jpeg.lib`), then make sure that  `LIBJPEG_INSTALL_DIRECTORY/bin` is on the path.

##### Grok static build with JPEG support (Windows)

`libjpeg-turbo` must be built with the `WITH_CRT_DLL` flag off, to ensure that the static version of the C runtime libraries is used.
