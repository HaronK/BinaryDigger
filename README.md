# Binary Digger

## Introduction

The idea of **BinaryDiger** (BD) is to help programmers inspecting unknown data formats and traversing quickly known ones.

BD uses plugin system for digging different data formats.
There are 2 different types of plugins: compiled and script based. Compiled plugin is a dynamic library that provides
specific interface for the main application to access hierarchical data.
Scripter (script based plugin) is also dynamic library which loads external scripts to parse data.

In current state Gui supports only view mode for data file.

## Screenshots

![Compiled plugin](images/BinaryDigger1.png "Compiled plugin")

![Lua scripter](images/BinaryDigger2.png "Lua scripter")

## Features

### Modularity

BD is built around interface implemented by plugins. Gui is a standalone application that loads plugins and uses their functionality to parse binary data into hierarchical structures.
Main requirement for the both parts of system (plugins and Gui) is supporting same interface.
In this case plugins and Gui are independent and could be developed separately.

### Compiled plugins

To add new compiled plugin just add new \*.cpp file into *BinaryDigger/plugins* folder and recompile project.
For examples look into existing plugin files.

#### Syntax

Plugin contains templates and plugin definition.

Template is defined by **TEMPL** and **TEMPL_END** macrosses.

```cpp
TEMPL(name, params...)

    // C++ code and template fields definitions
    ...
TEMPL_END
```

Where **name** is template name (without quotes) and **params** is parameters declaration similar to C-function parameters declaration.

To declare and use template elements several macrosses and methods could be used:

* **VAR** - simple element declaration.
 
```cpp
    VAR(WORD, size);
```

* **ARR** - array elemnt declaration.
 
```cpp
    ARR(CHAR, tag, 3);
```

* **VAL** - retrieve value of the POD element of the current template.
 
```cpp
    VAR(WORD,  entryNameLen);
    ARR(CHAR,  entryName, VAL(entryNameLen));
```

* To access subelement use following approach:
 
```cpp
TEMPL(T1)
    VAR(DWORD, len);
TEMPL_END

TEMPL(T2)
    VAR(T1, t1);
    DWORD_T = t1->item<DWORD>("len");
TEMPL_END
```

* To get or set file position use **getPosition** and **setPosition** methods:
 
```cpp
    bd_u64 pos = getPosition(); // retrieve current position
    setPosition(dataOffset);    // set new one
    ARR(UCHAR, data, 10);       // read data at those position
    setPosition(pos);           // return to the current position
```


There are several predefined templates for the POD types: **CHAR**, **UCHAR**, **WORD**, **DWORD**, **QWORD** and **DOUBLE**.

To make template available for using outside it should be registered:
 
```cpp
TEMPL(T1)
    VAR(DWORD, len);
TEMPL_END

PLUGIN(T1Plugin)
    TEMPL_REGISTER(T1);
PLUGIN_END
```

There should be at least one registered plugin.

### Scripters

Scripter (scripting plugin) doesn't provide any hardcoded templates. It uses script to generate parsed tree.

Gui application supports configurable highlighting in the script editor window.

> In current state Gui supports highlighting file only for Lua. This will be fixed in the future.

#### Syntax highlighting

Syntax highlighting subsytem uses Json format for configuration files. See example in [lua.json](gui/BinaryDigger/syntax/lua.json) file.

Configuration can contain several sections:

1. **regexps** - generic regular expression mappers. Each subelement contains:

2. **keywords** -

3. **comments** - 

4. **formats** -


#### Lua scripter


## Development tools.

- GUI: Qt.
- Plugins: plain C interface. Macroses are used to specify data templates/structures.
- Scripting: Lua. Script has only read access to the hierarchy tree.
- Build system: CMake.

[Installation instructions](INSTALL.md).


## Versions features.

### Version 0.1.0

- HEX view and synchronization with tree navigation (http://qt-apps.org/content/show.php/?content=133189)
- GUI with tree view
- defining templates as C macroses
- support of the minimum template types: char, short, int, int64, array, templ
- minimal Lua scripting support (console only)

### Version 0.1.1

- minimal Lua scripting support (Gui)
- configurable script syntax highlighting

### Version 0.X.0

- large files support (using memory mapped files or other techniques)
- multiple templates support: apply different templates to the hierarchy elements data (blobs)


## TODO

* Full Lua scripting support.
* Array as tree node. DONE
* *"size"* property of the node to specify predefined hardcoded node size. For the POD types generate "size" automatically.
* Postponed nodes loading/generation using "size" property. Additional parameter to specify deepness of tree generation.
* Node properties (size, color, toString, endian, ...).
