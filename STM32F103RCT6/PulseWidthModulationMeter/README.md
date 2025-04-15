# Overview

This project implements a PWM signal instrument designed to measure the frequency and duty ratio of input PWM signals, as well as generate PWM output signals with specified frequency and duty ratio parameters. The system utilizes STM32 microcontroller peripherals to achieve the functionality through two timers.

For signal generation, TIM2 is configured as a PWM generator with four output channels (PA0-PA3). The output frequency can be adjusted by pressing KEY0 (PC1) in 50Hz increments within a 50-200Hz range, while the duty ratio is modifiable via KEY1 (PC13) in 10% increments spanning 0-100%. The measurement functionality employs TIM3 in input capture mode through channel 1 (PA6), which analyzes incoming PWM signals to calculate their frequency and duty ratio parameters.

The system continuously updates the first four rows of the OLED screen in the main loop. Line 1 shows the output frequency, line 2 displays the output duty ratio, line 3 indicates the measured input frequency, and line 4 presents the input duty ratio. The implementation uses software-emulated I2C communication for the OLED display via GPIO pins, while button inputs utilize external interrupts for detection. Note that mechanical button bounce may occasionally cause multiple triggers per physical press.

# Environment

| Component | Version |
| --- | --- |
| Operating System | Windows 10 22H2 |
| Firmware Package (HAL Lib) | STM32Cube_FW_F1_V1.8.6 |
| STM32CubeMX | 6.11.1 |
| STM32CubeIDE | 1.16.0 |

# Wiring

| ST-LINK V2 | STM32F103RCT6 | 0.96 inch OLED screen |
| --- | --- | --- |
| SWDIO | PA13 |  |
| SWCLK | PA14 |  |
| 3.3V | 3V3 | VCC |
| GND | GND | GND |
|  | PB13 | SCL |
|  | PB15 | SDA |

# Usage

1. Open project: Open `.project` file in STM32CubeIDE.
2. Configure project: Configure the debugger according to the development environment.
3. Build: Build the project in STM32CubeIDE.
4. Run: Flash the code to the development board and run it.

# References

1. [jiangxiekeji's STM32 introductory tutorial](https://www.bilibili.com/video/BV1th411z7sn/?spm_id_from=333.788.videopod.episodes&vd_source=85e2a06878ef65c64ccb870d7dc21816&p=18)
