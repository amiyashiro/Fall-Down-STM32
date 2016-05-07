# Fall-Down-STM32

Final project for C335 (Computer Structures) course that explored
how programs are evaluated by a processor to control the physical
world.

For the final project I implemented a game called Fall Down in C on an
STM32 Discovery board. An explaination of the game and how it works
is contained in final_project.txt. Protocols such as SPI and
DMA are utilized to read data from an external SD card and control
an LCD screen, while I2C is used to used to read data from the
board's accelerometer.