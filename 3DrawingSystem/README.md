# 3DrawingSystem
Name: Abudureheman Adila  

### Program Description
This program is a implementation of 3D translation (specified by a 3D vector), 3D rotation about an arbitrary axis in 3D space (specified by two 3D points and a rotation angle in degrees), and 3D scaling (specified by one scaling factor used in all three directions). Scaling are done with respect to an object’s centroid. All objects are transformed with respect to their 3D world coordinates and implemented three orthographic projections (projecting all objects in the scene into the xy-plane, xz-plane, and the yz-plane). However for the simplicity, author have only provided with the one bunny textfile with all coordinates for user to play around. The scene should be written to a data file replacing the one the user have read initially.

### Operating System Check

If you are using MAC OS, please go to the Makefile to uncomment the OpenGL library for MAC (Line 6). And be sure to comment out the OpenGL for Linux/CSIF

### Steps to use my Two Dimentional Drawing Program  

Step 1: make  
Step 2: ./main  
Step 3: Select one of the options for operating my program   
1. Rotation  
2. Translation  
3. Scalling  
4. Exit

Step 4:  

##### if choice is 1:  
Enter angle such as 30   
Enter rotation axis seperated by space such as 0 0 0 1 1 1  
Enter which polygon would you be performing on such as 0,1,2.. (if these is more than one)

##### if choice is 2:  
Enter translation info seperated by space, such as: 2 2 2  
Enter which polygon would you be performing on such as 0,1,2..  (if these is more than one)

##### if choice is 3:  
Enter the scalling factor  such as 3  
Enter which polygon would you be performing on such as 0,1,2..  (if these is more than one)

##### if choice is 4: 
My program will save the previous result and quit  

##### Change the input file:
In my main function, first line is the input text name as  testScene.txt If you need to change, just change that one

## Thank you for using my program, have a great day!

