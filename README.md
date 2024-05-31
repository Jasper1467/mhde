# MHDE

MHDE is a modern C++ port of the Hacker Disassembler Engine (HDE). The current port leverages the features of the C++20 standard, providing a robust and efficient disassembly engine for modern applications.

## Features

- Utilizes C++20 features for enhanced performance and readability
- Compatible with both 32-bit and 64-bit disassembly
- Easy integration into existing projects

## Getting Started

### Prerequisites

Make sure you have the following installed on your system:

- CMake (version 3.21 or higher)
- A C++20 compatible compiler (e.g., GCC 10+, Clang 10+, MSVC 2019+)

### Building MHDE

To build MHDE, follow these steps:

1. **Clone the repository:**
    ```bash
    git clone https://github.com/Jasper1467/mhde.git
    cd mhde
    ```

2. **Create a build directory:**
    ```bash
    mkdir build
    cd build
    ```

3. **Generate build files using CMake:**
    ```bash
    cmake ..
    ```
    If you want to build the test files as well, add `-DBUILD_TESTS=ON`:
    ```bash
    cmake .. -DBUILD_TESTS=ON
    ```

4. **Build the project:**
    ```bash
    cmake -B .
    ```

After successfully building MHDE, you'll find the executable and any associated libraries in the `build` directory.

## Usage

### Example

Here is a simple example demonstrating how to use MHDE:

```cpp
#include "mhde.hpp"

int main()
{
    CMHDE mhde;
    unsigned char code[] = { 0x90, 0x90, 0x90 }; // NOP instructions
    unsigned int result = mhde.Disassemble(code);

    if (result == 1)
        std::cout << "Disassembly successful: 1 byte processed." << std::endl;
    else
        std::cerr << "Disassembly failed." << std::endl;

    return 0;
}
