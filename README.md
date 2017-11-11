# KinectXbox360-UE4
kinect Xbox 360 sdk 1.8 Plugin for Unreal Engine 4

## Installation
- clone the code into the "Pluings" folder of your c++ ureal engine project (ex. myProject\Plugins\KinectXbox360-UE4)
- run, compile project as usual

## info drivers kinect xbox
In microsoft windows should detect the device and install the drivers by itself.
</br>
## Example Usage


### Get Sensor and Initialize
---------------------------------------

![Capture1](http://aledel.github.io/KinectXbox360-UE4/Images/Capture1.jpg)

- Kinect node is a functionLibrary, this return the Class *UKinectManager*.
- Get Sensor is a method of that class and returns an array of connected Kinect (*UKinectSensor* class),
  in this example I take the first sensor found.
- Now we can initialize the sensor:
  - We need to pass a base material to apply later the rgb and deep textures.
  - The BaseMaterial need a *Textureparameter* named **KinectTexture**. (used to apply the buffer when creating the instance dynamic material.)
    
    ![Capture4](http://aledel.github.io/KinectXbox360-UE4/Images/Capture4.JPG)




### Get and Update Color and Depth Stream
---------------------------------------

![Capture2](http://aledel.github.io/KinectXbox360-UE4/Images/Capture2.jpg)

- We use the method *UpdateColor* and *UpdateDepth*. (look at the picture above, also each material has been applied to Cube and Cube1.)



### Update Skeleton Stream and get Joints Positions and Bone Rotations
---------------------------------------

![Capture3](http://aledel.github.io/KinectXbox360-UE4/Images/kinect_bones_detail.jpg)

- The function *UpdateSkeleton* set a structure array variable (skeletons detected). 
  - Each structure contains:
    - *JointPosition* (array Vector)
    - *Rotation Bones* (array Rotation)
    - *PlayerID* (Skeleton ID)

## Example bones
[Kinect_Skel.zip](http://aledel.github.io/KinectXbox360-UE4/examples/Kinect_Skel.zip)

<p float='left'>
	<br>
	<img width="500" src="http://aledel.github.io/KinectXbox360-UE4/Images/bones.jpg" alt="bones">
	<br>
	<br>
	<br>
</p>



