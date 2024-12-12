# Lab Notebook

## September 11, 2024
1. Discussed the core goals of our project:
   - The Robot should autonomously navigate the water body and detect and pick up trash.
2. Researched existing water-skimming robots for design inspiration.
   - Reasearched for similar products in the market like the Jellyfish bot
3. Began investigating water turbidity as an indicator of pollution:
   - Discovered that the way to measure water clarity is called turbidity and could be a neat add-on to teh project without increasing difficulty too much 
4. Collaborated to create the RFA, focussing on presenting the problem, solution, and technical approach.

## September 12, 2024

### Objective
   - We finalized the title: **Water-Skimming Robot for Pollution Cleanup**. The project focuses on building a robot that skims water to collect small debris, using autonous navigation and provides real-time feedback on water quality using turbidity.
   - The post outlined the key points:
 	- **Problem**: Water pollution from trash, invasive species, and poor waste management is a serious threat to aquatic ecosystems and public health. Current cleanup methods are inefficient and labourious.
 	- **Solution**: A small robot that uses GPS and sensors to stay in a boundary, collects debris in a net, like a roomba, and measures water clarity using a turbidity sensor. LEDs (green or red) give instant feedback based on the water’s pollution level.
   - **Subsystems**:
 	- **Motor Control**: We’re using dual brushless DC motors (LICHIFIT RC Jet Boat Underwater Motor Thrusters) to propel the boat, controlled by the microcontroller.
 	- **Autonomous Navigation**: A servo motor for steering, guided by a GPS module (NEO-6M) and IMU. This setup will help the robot turn quickly and return to the starting point to drop off debris.
 	- **Power**: A 7.4V Zeee battery to power everything, all enclosed in a waterproof case with a voltage regulator for the rest of the system.
 	- **Chassis and Storage**: Lightweight materials (PVC pipes and styrofoam) for the frame, with a plastic net attached to catch trash.
 	- **Sensors**: We will use a turbidity sensor, which measures how cloudy or clear the water is. A GPS to know the location of the robot and an IMU for accelerometer, gryoscope and compass.
   - **Success Criteria**:
 	- The robot can navigate a small body of water (like a pool or a lake) without crossing boundaries.
 	- It collects floating debris and brings it back to the disposal area.
 	- The turbidity sensor correctly triggers the green or red LEDs based on water clarity.

## September 16, 2024
1. **Set Up the Proposal**
   - Worked together wuth teammates to create the overall layout, including details about the problem, solution, and subsystem components.

2. **High-Level Requirements**
 	1. Autonomous Navigation
    The robot must be able to autonomously navigate a predefined water source without
    crossing boundaries that we will set. It should detect these boundaries using GPS and
    IMU data with an accuracy of 10 feet to show proper coverage of the water surface.
    2. Debris Collection and Return:
    The robot must detect and collect floating debris using its skimming net. Every 5
    minutes, it should be able to hold and transport at least 250 grams of debris and return to
    a predefined coordinate with the same accuracy of 12 feet.
    3. Water Quality Feedback:
    The system must monitor water clarity using a turbidity sensor. Real Time feedback will
    be provided by LED lights, where green indicates acceptable water quality (the turbidity
    is below 50 NTU) and red signals unacceptable water quality (the turbidity is above 50
    NTU).
    4. Reach goal: If we have time, we can add pollutant object detection to steer towards
    floating debris captured via a camera in real time. This would require incorporating an
    OpenCV Convolutional network into our pre-existing control algorithm. Being able to
    identify and steer the chassis towards floating trash within 5 feet of the front of the robot
    would be a stretch goal.




## September 17, 2024
Met the TA and broughr the requirements for the meeting:
- A draft of our block diagram.
- The  high-level requirements
- One subsystem requirement, the turbidity sensor and feedback system:
- The sensor must  measure water clarity in NTU, triggering LEDs basedon the water clarity.
- Discussed the reach goal and decided to remove it


## September 19, 2024
Worked together with the group to submit the project proposal due that night, worked as all parts the needed help without picking a speciality

## September 24, 2024
Met with our TA to discuss the feedback on our project proposal to submit a better version for the regrade.

Recieved input:
   - We need some sort of calculation or evidence to prove feasability.
   - We need to provide visual Aid to better explain the design to reader.


## October 1, 2024

Met with the TA to review feedback on our submitted proposal and plan next steps for revisions and building the boat.

We discussed meeting with the machine shop as we had technically already  missed the deadline and decided to prepare a CAD model and provide to the machine shop.


## October 2, 2024

Teammate spoke with the machine shop and shared the options they gave us for building the boat:
The design will consist of two pontoons connected by a bridge, the electronics box will be on top of the bridge
Motors will be attached to the ends of the pontoons for propulsion. There was confusion as to use PVC of foamfor the pontoons and eventually PVC was decided as we did not want to accidentally pollute the water body while trying to clean it.


## October 3, 2024
Worked together with the group to submit the design document due that night, with Turbidity taken by Zach, me and DJ worked on all parts remain sub-systems togther without sliptting them up.


## October 4, 2024

Worked with teammates to update the Project proposal with the TA's inputs for the regrade.

## October 7, 2024

Met with Gregg Bennett at the machine shop to provide the CAD drawing by DJ and discuss the feasibility of the  design.
We were informed to provide the specific conponents to the machine shop ASAP to start manufracturing the boat

## October 22, 2024
Met with TA to discuss all our parts arriving except the motors and decided to find a replacement part and order a new motor


### Objective
Address the missing motor issue and decide on a replacement. Plan next steps based on feedback from the TA.
Decided to work on sensors and we new that calibration would take a lot of time and would require 1 person to work on it.

Page 16
## October 25, 2024

Assembled the circuit for the project as a group using the KiCad schematic as a guide
Started writing to basic code but was unable to test.

## October 28, 2024
Worked on testing the Code and sensors all weak getting the IMU magnetometer to work with limited reliability and spent all week calibrating it to no avail, eventually realized that I would use the raw x and Y magnitometer readings to try improve calibration

##  November 5, 2024
Spent all week working on getting the IMU code to work. Evenntually realised that IMU calibration woulf be very difficult as it was working as a mix between a compass and a metal detector and hence moving to new locations in the lab every time ruined previous calibrations.


## November 8, 2024
Completed the revision of the design document, working with all teammates and contributing wherever possible.


## November 11, 2024
Tried to help teammate with issues regarding the ESC, worked as a group but were unable to fix the error

## November 15, 2024
Met with the machine shop to deliver all components like the motors, ESCs anf turbidity sensor, confirmed the design with the machine shop techs and reminded then of the LEDs required on top of the box

## November 19, 2024
Finally got the GPS code to work and found that the accuracy is roughly 10 feet regardless of whether stationary or mobile, did not take any measurements and did not think they were nessecary at that point.

## November 29, 2024
Picked up the boat from Zach and decided to reattempt IMU calibration but this time with the Boat in place to try to complete calibration. Recorded GPS data for final demo

## Dec 3, 2024
Give the Final demo and decided with the team to work on the mock presentation next.

## Dec 6, 2024
Worked on and delivered the mock presentation

## Dec 8, 2024
Recorded IMU to show tthe difficulty in its calibration

## Dec 10, 2024
Gave the Final presentation

## Dec 11, 2024
Worked on the Final paper and worked on all parts of the paper together with my teammates.