# Concept

The purpose is to provide C++ event managing tools.

- event_listener
- event_manager
- event_box

# Install

## Requirements

Binaries:
- A C++20 compiler (ex: g++-10)
- CMake 3.16 or later

Testing Libraries (optional):
- [Google Test](https://github.com/google/googletest) 1.13 or later  (optional)

## Clone

```
git clone https://github.com/arapelle/arba-evnt --recurse-submodules
```

## Quick Install

There is a cmake script at the root of the project which builds the library in *Release* mode and install it (default options are used).

```
cd /path/to/arba-evnt
cmake -P cmake/scripts/quick_install.cmake
```

Use the following to quickly install a different mode.

```
cmake -P cmake/scripts/quick_install.cmake -- TESTS BUILD Debug DIR /tmp/local
```

## Uninstall

There is a uninstall cmake script created during installation. You can use it to uninstall properly this library.

```
cd /path/to/installed-arba-evnt/
cmake -P uninstall.cmake
```

# How to use

## Example - Connect a function to an *event_manager*

```c++
#include <arba/evnt/evnt.hpp>
#include <iostream>

class int_event
{
public:
    int value;
};

int main()
{
    evnt::event_manager event_manager;
    event_manager.connect<int_event>([](int_event& event)
    {
        std::cout << "I received an int_event: " << event.value << std::endl;
    });

    event_manager.emit(int_event{ 42 });
    return EXIT_SUCCESS;
}
```

## Example - Using *arba-evnt* in a CMake project

See *basic_cmake_project* in example, and more specifically the *CMakeLists.txt* to see how to use *arba-evnt* in your CMake projects.

# License

[MIT License](./LICENSE.md) © arba-evnt
