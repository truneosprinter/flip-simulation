# FLIP Simulation in the Terminal using C + termios for input

This is a simple single-file C program that I created in order to better understand the FLIP (FLuid Implicit Particle) simulation created in the 1980s to simulate plasma flow - in this case I have instead repurposed it to create an interactive fluid flow program.

In terms of inputs:
- arrow keys can be used to move the cursor
- 'a' can be used to add more particles
- 'r' can be used to reset the simulation environment

Please note that I have set the maximum particles to 100,000 due to the power of my computer, however on lower end systems this could lead to crashing of the program - please feel free to change this if you are trying out the code for yourself!

The code can be ran in 2 modes: terminal mode, and SDL mode. They are NOT identical. On a posix system:
- For the terminal mode, simply install make, and run ```make run-term```
- For the SDL mode, make sure you have installed sdl2, along with sdl2_ttf, and then run ```make run-sdl```

When running the code, feel free to tweak the constants at the beginning of either program; this will result in different viscosities, gravities, and other properties of the simulation!

This project was entirely developed by me as of 2025, please enjoy using it!
