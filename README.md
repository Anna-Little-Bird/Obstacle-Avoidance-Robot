# Obstacle Avoiding Robot

![obstacle avoiding robot](https://github.com/Anna-Little-Bird/Obstacle-Avoiding-Robot/blob/master/Photos/1_small.jpg)

The goal of this project is to design the autonomous robot which will be able to perform the following tasks:
-	navigate itself;
-	avoid obstacles during navigation;
-	display states of the systems on a LCD display;
-	minimal size and cost.

The core of the project is PIC18F25K22 microcontroller. For navigation task two ultrasonic sensors HC-SR04 were used. The moving part was presented by 2 wheel chassis which was operated by PIC AXE408 motor shield.

To fulfil the requirements the following state diagram was designed:
![state diagram](https://github.com/Anna-Little-Bird/Obstacle-Avoiding-Robot/blob/master/Photos/State%20Diagram.jpg)

There are 5 states: IDLE, RUN, TURNRIGHT, TURNLEFT and BACK. In general the Robot is moving forward when there is no obstacle to the left and right of it in less than 20 cm distance. If there is one the Robot will move according to the measured distance.
