# Project Overview

This project is a Game Boy emulator that runs on a Raspberry Pi Pico and displays the output on a Watara Supervision handheld console.

The project is written in C/C++ and uses the following main technologies:

*   **Raspberry Pi Pico SDK:** For interacting with the Raspberry Pi Pico hardware.
*   **Peanut GB:** A single-header Game Boy emulator library.
*   **CC65:** A cross-compiler suite for 6502 based systems, used here to compile code for the Watara Supervision.

The project is structured as follows:

*   `src/`: Contains the source code for the Game Boy emulator running on the Raspberry Pi Pico.
*   `cc65/`: Contains the source code for the Watara Supervision.
*   `utils/`: Contains utility scripts, including a script to convert the Watara Supervision ROM into a C header file.
*   `CMakeLists.txt`: The main build script for the project.

# Building and Running

The project is built using CMake. The following steps are required to build the project:

1.  **Install the Raspberry Pi Pico SDK.**
2.  **Install the CC65 compiler.**
3.  **Set the `PICO_SDK_PATH` and `CC65_HOME` environment variables.**
4.  **Run CMake to generate the build files.**
5.  **Run make to build the project.**

The output will be a UF2 file that can be flashed to the Raspberry Pi Pico.

```bash
# TODO: Add the exact build commands here.
mkdir build
cd build
cmake ..
make
```

# Development Conventions

The code is written in C/C++ and follows the conventions of the Raspberry Pi Pico SDK. The code is well-commented and easy to understand.
