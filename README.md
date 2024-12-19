# ARM GPIO Project

This project involves working with ARM assembly and C programming to manipulate GPIO pins on a Raspberry Pi. The project includes multiple C and assembly files, each serving different purposes.

## Files

### `DISPLAYPLOT.C`

This C file reads LED frequency and duty cycle data from a CSV file and generates graphs using gnuplot.

#### How to Compile and Run
1. Ensure `WaveLED.csv` is in the same directory.
2. Compile the program:
    ```sh
    gcc -o displayplot displayplot.c
    ```
3. Run the program:
    ```sh
    ./displayplot
    ```

### `NEWSTUDENT.C`

This C file controls the blinking of LEDs connected to the Raspberry Pi GPIO pins using the WiringPi library.

#### How to Compile and Run
1. Install WiringPi:
    [WiringPi Setup](https://learn.sparkfun.com/tutorials/raspberry-gpio/c-wiringpi-setup)
2. Compile the program:
    ```sh
    gcc -o Newstudent Newstudent.c -lwiringPi -lpthread
    ```
3. Run the program:
    ```sh
    ./Newstudent
    ```

### `Q2.C`

This C file contains a function to reverse a string using recursion.

#### How to Compile and Run
1. Compile the program:
    ```sh
    gcc -o q2 q2.c
    ```
2. Run the program:
    ```sh
    ./q2
    ```

### `Q2.S`

This ARM assembly file contains the implementation of the reverse function and the main function to reverse a string.

#### How to Assemble and Run
1. Assemble the program:
    ```sh
    as -o q2.o q2.s
    ```
2. Link the program:
    ```sh
    ld -o q2 q2.o
    ```
3. Run the program:
    ```sh
    ./q2
    ```

## Prerequisites

- WiringPi library for GPIO manipulation.
- GNU Compiler Collection (GCC) for compiling C programs.
- GNU Assembler (GAS) for assembling ARM assembly programs.
- gnuplot for generating graphs.

## Useful Commands

- Check WiringPi version:
  ```sh
  gpio -v
  ```
- Check GPIO status:
  ```sh
  gpio readall
  ```
- Check Raspberry Pi version:
  ```sh
  cat /etc/os-release
  ```

## GPIO Pin Connections

- Green LED: GPIO 27
- Red LED: GPIO 13
- Ground: GND
- GPIO 14 to Monitor GPIO 15
- GPIO 15 to Monitor GPIO 14

## Raspberry Pi Version

- Raspbian GNU/Linux 10 (buster)

## License

This project is licensed under the MIT License.
