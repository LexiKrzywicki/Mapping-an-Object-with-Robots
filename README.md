# Mapping an Object with Robots

## Overview
The goal of this project was to map an object in the middle on the field. The team did this by use an ultrasonic sensor, IR sensor, encoders, and a camera. The team also created a PD controller to keep the robot at a specific distance from the wall, and a PI controller to maintain a specific velocity. As the robot was wall following, the  ultrasonic sensor took readings every 250 ms and stored them in an array. Once all the reading from one side were taken, the data was sent to an MQTT server via the ESP32. A visualization of the object in the field was created from the array of readings.

## Sensors: 
  **Rangefinder:** 
The rangefinder was used to determine the distance between the ROMI and the objects on the interior of the arena.  The rangefinder does so by sending out ultrasonic pulses and reporting the time it took the pulses to bounce back to the rangefinder. Because the speed of sound at sea level is a known constant the distance between the object and the rangefinder can be calculated.  
  
  **Infrared Sensor:** 
The infrared sensor was used to measure the distance from the wall. It was placed vertically at a 45 degree angle on the robot so that when the robot turned slightly, the the distance measured would either increase or decrease. The robot can used the value read form the IR sensor along with a PD controller to drive parallel at a specified distance.
  
  **Encoders:** 
The encoders can provide information about the position and speed of a robot. There are 1440 counts per one wheel revolution. The count increases or decreases when the electrical signal is read. The position can be checked from the calculated wheel circumference and velocity can be determined using time that has passed.
  
  **Camera:** 
The camera was used to check for the distance from the specified April tags. The distance from an April tag can be determined by how many pixels the April tag takes up. The more pixels the april tag fills, the closer the robot is to the tag.


## Controllers:
  **PD Controller:** 
  The wallFollow() function takes in the a distance, Kp, Kd, target velocity and time. The distance from the wall is measured using the IR sensor, and the error is the desired distance minus the measured distance. The delta error of the distance from the wall is the current error minus the previous distance over the time that has elapsed from the previous distance measurement. The deltaVelocity = Kp * the distance error - Kd * the delta error of the distance. The new velocity for each wheel is the target velocity +/- half of the delta velocity. 
  
 **PI Controller:**   
The velocity of the wheels are then set using the process method in the speedController class. Process() takes in a velocity for the left and right motors. It checks the target velocity with the calculated actual velocity from the time passed and encoder count. The effort that the motor is set to is Kp * the different in target velocity and actual velocity +  Ki * the current error and the previous error.

## State Machine:

<img width="422" alt="Screen Shot 2022-05-02 at 3 33 52 PM" src="https://user-images.githubusercontent.com/80935708/166313358-2274c2d6-10bb-463a-ba9b-fea8748b0e90.png">


## Mapping:
In order to send data to our visualization code we used an MQTT server connected to our ESP32. The ESP32 was connected to the ROMI using UART communication. The ESP32 did not send any information back to the ROMI so in order to save ports for communication with the sensors the ESP32 TX was not connected to the ROMI RX. The data sent through the ESP32 was collected using odometry (forward kinematics) and the rangefinder. Each state (North, East, West, South) had their own functions for handling the computing and sending of the information to the MQTT server called *handleDirection*. The functions for handling the data were all similar to each other but had minor differences to indicate the difference between each side for the visualization code and the target theta of each direction. All of the *handleDirection* functions calculated the actual position of the object sensed by the rangefinder using the error between the actual theta value and teh target theta value and the ROMI's position in the area. The rangefinder data was filtered with a median filter that collected 5 data points and returned the median. These readigns were also filtered within the *handleDirection* functions by throwing out any readings that would be outside the areana or in the path of the ROMi's driving.

## Visualization:
This program uses a file called Map_Data.txt that should be located in the Mapping_Visualization folder. The program reads the file line by line. Each line should be in the format “(x,y) = z” where x is the x coordinate, y is the y coordinate, and z is the value representing which side of the field the romi is traveling on. The program then populates a 20 by 20 2D int array (map) with the side of the field value in the location corresponding to the (x,y) position. After that the 2D array (map) is passed into the constructor of MappingJFrame. 


The MappingJFrame Class makes a new JFrame object when its constructor, which takes in the 2D array (map), is called. The constructor runs the overridden paint function. Within the paint function defined in the class, the program traverses the map array and draws rectangles at each position. The color of the rectangle is determined by the values at that location in the map array. The four sides of the field (1,2,3,4) are represented by red, yellow, blue, and green rectangles respectively, and a 0 (or empty location) is represented by a white rectangle.

The MappingJFrame class also provides the option to select which sides of the field to display measurements from. This is done by the booleans at the top of the class (drawRed, DrawBlue, etc) as well as one boolean to show all sides regardless of the individual booleans. This feature is great for debugging so you can see which side is giving you problems.

## Improvement:
If the team were to improve on this project, the team would either use the camera to drive straight towards the April tag or improve the wall follower PD controller. The team would also improve the filter by taking in more values into the array to average to get a more accurate average.
