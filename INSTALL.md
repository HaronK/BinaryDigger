# Instalation instruction

Currently binary packages are not provided. You can compile project from the source code.

Project was compiled and tested only on Ubuntu 13.10, but for the other Linux platforms it should be also compilable.

## Dependencies

### Qt

Gui part of the BinaryDigger is QtCreator project. Download and install it from [here](http://qt-project.org/downloads).

### PocoProject

Plugin system depends on [PocoProject](http://pocoproject.org/). Download and install it from [here](http://pocoproject.org/download/index.html).

### Lua and LuaBind

LuaScripter plugin depends on [Lua](http://www.lua.org/) and [Luabind](http://www.rasterbar.com/products/luabind.html). You can download and install them from here:

* [Download Lua](http://www.lua.org/download.html)

* [Download Luabind](http://sourceforge.net/projects/luabind/files/luabind/)

or on the Linux platforms you can use your package manager.

## Compilation

### Gui

Open project in QtCreator and compile it (*BinaryDigger/gui/BinaryDigger/gui/BinaryDigger.pro*).

### Plugins

Change *BinaryDigger_GuiBuildFolder* variable in *BinaryDigger/CMakeLists.txt* to point to Gui build folder on your system.

```
cd BinaryDigger/build
cmake ..
make
```
