
--------------------------------------------------------------------------------------------

COMPANY		: Purdue University Northwest Department of Computer Information Technology								
DESCRIPTION	: CM1K Parallel Interfacing with Arduino-Due Micro controller
VERSION		: 1.0 (05/7/2015) 

http://www.ricardocalix.com/researchfiles/mlhardware/mlhardware.htm
--------------------------------------------------------------------------------------------

----------------------------------------------

Adding CM1K Parallel libraries to your Arduino

----------------------------------------------

Method 1

1.	Open Arduino
2.	Sketch -> Import Library -> Add Library 
3.	Select "CM1KParallel" -> Open
4.	Verify Library is loaded by checking list of available libraries as follows
5.	Tools -> Sketch -> Import Library -> "Check the list"
6.	Repeat above steps for both "CognimemParallel" and "CognimemTest" libraries



Method 2

1.	Copy "CM1KParallel", "CognimemParallel" and "CognimemTest" library folders 
2.	Browse Arduino library folder ( default: Documents\Arduino\libraries\)
3.	Paste All three library folders
4.	Verify all libraries are loaded by checking list of available libraries as follows
5.	Open Arduino -> Tools -> Sketch -> Import Library -> "Check the list"



For information on installing libraries, see: 
http://arduino.cc/en/Guide/Libraries

--------------------------------------------------
Running CognimemTrainAndTest

--------------------------------------------------

1.	Connect Arduino Due USB
2.	Select Arduino Due Board 
	Tools -> Board -> "Arduino Due (Programming Port)"

3.	Select Arduino Due COM Port
	Tools -> Port -> "COMX(Arduino Due(Programming Port))"

4.	Open CognimemTrainAndTest.ino 
5.	Choose KNN or RCE by setting EN_KNN to 1 (KNN) or 0 (RCE).
	Default is RCE (0)
	Ex: To Enable KNN
	int EN_KNN = 1;


6.	Define Train File Name (UPPERCASE and less than 8 character long)
7.	Define Test File Name (UPPERCASE and less than 8 character long)
8.	Define k. Number of neurons fired (Nearest Neighbours)

9.	Save all the modification 
	File -> Save 


10.	Verify/Compile
	Sketch -> Verify


11.	Upload to Arduino DUE
	File -> Upload


12.	Once upload is completed, Open Serial Monitor

	Tools -> Serial Monitor (Use 9600 baud)