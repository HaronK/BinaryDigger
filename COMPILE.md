
# BinaryDigger compilation

## Linux

### Dependencies

1. CMake, OpenGl and Luabuind packages. On Ubuntu:
```
$ sudo apt-get install cmake mesa-common-dev libglu1-mesa-dev libluabind-dev
```

  **Tip**. To let Luabind work with the latest versions of Boost one should wrap definition
  of **LUABIND_OPERATOR_ADL_WKND** macro in */usr/include/luabind/object.hpp* file in **#if/#endif**
  block:
```cpp
#if BOOST_VERSION < 105700
...
#endif
```
  See [here](https://github.com/eglaysher/rlvm/commit/373a3db1c4d3c9a4b9eb60b8bca60fa58d1687f9)
  for details.
  
  Create link to **liblua5.2.so**:
```
sudo ln -s /usr/lib/x86_64-linux-gnu/liblua5.2.so /usr/lib/liblua.so
```

2. [Boost](http://www.boost.org/). Download latest version and install. Example for version 1.59:
```
$ wget http://downloads.sourceforge.net/project/boost/boost/1.59.0/boost_1_59_0.tar.gz
$ tar -xzf boost_1_59_0.tar.gz
$ cd boost_1_59_0
$ ./bootstrap.sh
$ sudo ./b2 -j 4 variant=release install
```

3. [Qt](https://www.qt.io/). Install latest Qt. Set **$QT_HOME** environment variable to the root
   folder of installed Qt package.
   This is optional package. If it is not installed than **Gui** library will not be compiled.

### Compilation

```
$ cd BinaryDigger
$ mkdir build
$ cd build
$ cmake ..
$ make -j 4
```
