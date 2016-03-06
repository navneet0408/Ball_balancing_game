
Application Description:
The application interfaces MPU6040 with the Galileo board and reads the angular velocity and acceleration from it to calculate the current angle. This, in turn, is used to light an led on the LED grid, and move the lighted led in the direction of tilt.
The application requires the user to hold the MPU6050 chip at an angle of zero in the beginning of the level. The start of the first level is marked by one expanding ring of LEDs. Once the level is completed, the next level is started. The start of the second level is marked by two expanding LED rings. The start of the third level is marked by three expanding rings, and so on. The difficulty increases with the increase in levels. Once we lose, we come back to level 1.

Compiling and running the Application:
The source code consists of two files, MPU_UserApp.c and MPU_Driver.c. The latter is the kernel module used for the application, and the former is the user application. The source code also has a makefile to compile it.
For compilation, we need to go to the source directory and type:
make
For running the application, we need to compile and the type in the following:
insmod MPU_Driver.ko
./MPU_UserApp