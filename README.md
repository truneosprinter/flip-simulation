# FLIP Simulation in the Terminal using C + termios for input

This is a simple single-file C program that I created in order to better understand the FLIP (FLuid Implicit Particle) simulation created in the 1980s to simulate plasma flow - in this case I have instead repurposed it to create an interactive fluid flow program.

In terms of inputs:
- arrow keys can be used to move the cursor
- 'a' can be used to add more particles
- 'r' can be used to reset the simulation environment

Please note that I ahve set the maximum particles to 100,000 due to the power of my computer, however on lower end systems this could lead to crashing of the program - please feel free to change this if you are trying out the code for yourself!

In order to compile, simply run ```make run``` in the cloned directory

This project was entirely developed by me as of 2025, please enjoy using it!
