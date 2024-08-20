To compile and run the STANDARD Animal project, do the following:

1 - make sure you are in this directory: ~/22804897-rw214-project/src

2 - then type the following into the command-line: javac -d ../bin Animal.java
    this is so that the .class files are directed into a different folder (i.e. the bin folder)
    after compiling.

3 - ignore this message after compiling: 
"Note: ./Picture.java uses or overrides a deprecated API.
 Note: Recompile with -Xlint:deprecation for details."
 This is just because I'm using an outdated Picture.java file I believe but it still works fine.

4 - To run the program successfully make sure that the second arguement has the file path included with the file name/image name like such, "../tests/input/input/<file name>".

-------------------------------------------------------------------------------------------------

To compile, run and use the GUI MODE successfully:

TO COMPILE: 
	1. make sure you are in this directory: ~/22804897-rw214-project/src

	2. then type the following into the command-line to compile and transfer the .class 
	files to the bin folder: javac -d ../bin AnimalGUI.java

	3. ignore this message after compiling: 
	"Note: ./Picture.java uses or overrides a deprecated API.
 	Note: Recompile with -Xlint:deprecation for details."
 	This is just because I'm using an outdated Picture.java file I believe but it still 
 	works fine.
-----------------------------------------------------------------------------------------------
TO RUN: 
 	4. First make sure you are in this directory: ~/22804897-rw214-project/bin
 
 	5. Then, before running the program make sure to give the file path as well as the 
 	file name as the first argument like such: 
 	java AnimalGUI ../tests/input/input/cheetah1.jpeg
-----------------------------------------------------------------------------------------------
TO USE: 
	6. The original image will now be displayed in a new window. To display the greyscaled 
	image, press the '0' key. When pressed the following message will be displayed in the 
	terminal, "Displaying the greyscaled image" and the current image will be replaced 
	with the greyscaled image.
 
	7. To display the noise reduced image, press the '1' key. When pressed the following 
	message will be displayed in the terminal, "Displaying the noise reduced image" and 
	the current image will be replaced with the noise reduced image.

	8. To display the edge detected image, press the '2' key. When pressed you will be 
	prompt to enter in the epsilon value into the terminal. After which, the following 
	message will be displayed in the terminal, "Displaying the edge detected image with
	an epsilon value of <epsilon>" and the current image will be replaced with the edge 
	detected image.

	9. To display the spot detected image, press the '3' key. When pressed you will be 
	prompt to enter in the epsilon value, the lower radius limit value and then the upper
	radius limit value into the terminal. After which, the following message will be displayed
	in the terminal, "Displaying the spot detected image with an epsilon value of <epsilon>, 
	a lower radius limit of <lowerLimit> and a upper radius limit of <upperLimit>"
	
	10.To quit and close the program, you can press the 'q' key.
-----------------------------------------------------------------------------------------------
