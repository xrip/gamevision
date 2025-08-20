# GameVision: Game Boy Emulator for Watara Supervision


[![GameVision running Wario Land](https://img.youtube.com/vi/Bpv42_TeBiA/0.jpg)](https://www.youtube.com/shorts/Bpv42_TeBiA)

**Bring your old Watara Supervision back to life with the power of the Raspberry Pi Pico!**

GameVision is a Game Boy emulator that runs on a Raspberry Pi Pico and uses the Watara Supervision as a display and controller. This project aims to give a new purpose to this old and forgotten handheld console.

## Features

*   **Game Boy and Game Boy Color emulation:** Play your favorite Game Boy and Game Boy Color games on the Watara Supervision.
*   **Sound support:** Enjoy the original game music and sound effects.
*   **Low power consumption:** The Raspberry Pi Pico is a low-power microcontroller, making it ideal for this project.
*   **Open source:** The entire project is open source, so you can contribute to its development.

## Hardware Requirements

*   **Purple Chinese RP2040 clone:** This is strictly needed. You can find it on AliExpress.
*   **Watara Supervision:** The display and controller for the emulator.
*   **watapico Watara flash cartridge:** The hardware used to connect the RP2040 to the Watara Supervision. You can find the details on the [watapico GitHub repository](https://github.com/xrip/watapico).

## RP2040 Pinout

The following table shows the pinout for the Raspberry Pi Pico:

| Pico Pin | Function         | Supervision Pin |
| :---: | :---: | :---: |
| 1-17     | Address Bus (A0-A16) | 1-17            |
| 18-25    | Data Bus (D0-D7) | 18-25           |
| 29       | /RD              | 29              |

## Building and Flashing

The project is built using CMake. The following steps are required to build the project:

1.  **Install the Raspberry Pi Pico SDK.** You can find the instructions on the [Raspberry Pi website](https://www.raspberrypi.com/documentation/pico/getting-started/).
2.  **Install the CC65 compiler.** You can download it from the [CC65 website](https://cc65.github.io/).
3.  **Set the `PICO_SDK_PATH` and `CC65_HOME` environment variables.**
4.  **Clone the repository:**

    ```bash
    git clone https://github.com/your-username/gamevision.git
    ```

5.  **Create a build directory:**

    ```bash
    mkdir build
    cd build
    ```

6.  **Run CMake to generate the build files:**

    ```bash
    cmake ..
    ```

7.  **Run make to build the project:**

    ```bash
    make
    ```

8.  **Flash the UF2 file to the Raspberry Pi Pico.** You can do this by holding down the BOOTSEL button on the Pico while plugging it into your computer. It will then appear as a mass storage device, and you can drag and drop the UF2 file onto it.

## Peanut GB

This project uses the [Peanut GB](https://github.com/deltabeard/Peanut-GB) emulator library. The source code for the library is included in this repository.

## Contributing

Contributions are welcome! If you have any ideas, suggestions, or bug reports, please open an issue on the [GitHub repository](https://github.com/your-username/gamevision/issues).

## Credits

+ Hardware design: Sa Gin
+ Software: xrip, [vrodin](https://github.com/vrodin)
+ Beta testing: Oleg

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
