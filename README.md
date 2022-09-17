# operating_stystem_ex3

## ex31 
A program that accepts paths to two files as arguments to main, and checks whether the two files Identical/similar/different. The paths include the filenames.
If the files are identical (contain exactly the same content), the program will return 1, if the files are similar (explanation below) the program will return 3, otherwise, if the files are different, it will return 2)

Identical files are files in which all characters are equal. <br/>
Similar files are files that are not identical, but contain the same text, and there is a difference in the use of small or large letters, space and/or line breaks.
Different files are files that are neither identical nor similar.<br/>

<br/>
For example - these files are similar:<br/>
File A: <br/>
12ab23 <br/>
File B: <br/>
12Ab23 <br/>
File C: <br/>
12aB23 <br/>
File D: <br/>
12AB23 <br/>
File E: <br/>
12 aB 23 <br/>
File F: <br/>
12 <br/>
ab2 <br/>

### running example:
[os2012@localhost ~]$ ./comp.out /home/os2012/code/1.txt /home/os2012/code/2.txt <br/>
[os2012@localhost ~]$ echo $? <br/>
3 <br/>


## ex32
A program that accepts a path to a configuration file as an argument to main. <br/>
It can be assumed that the file exists (and it doesn't a folder). <br/>
The configuration file contains 3 lines: <br/>
- Line 1: Path to the folder containing subfolders. At the first level (one in), each folder represents
user, and should contain a c file.
- Line 2: Path to the file containing input.
- Line 3: Path to the file containing the correct output for the input file from line 2.
The configuration file will end with a newline character.
The program enter all subfolders (and ignore other files that are not folders, if applicable and exist. Within the folder from line 1, search in each of its subfolders (and not at deep levels
more) c file, and compile it. The created runtime file must be run with the input that appears in the file at the location in line 2.


There will be at most one c file in the folder (there may not be a c file at all, and there may also be files and folders of other types, except for files with the extension out.)
It can be assumed that the c file will necessarily have a suitable extension, for example file.c. The name of a file has no meaning <br/>



The output of the program compared with the desired output file, whose location comes from line 3, using the program comp.out that I implemented in part A of the exercise.
The program creates a file (in the folder from which your program was run) called results.csv which contains for each username (subfolder name), its score (between 0 and 100) according to the answer that
comp.out returned , and the reason. The character "," must be written between the username and the grade, and the reason his (without spaces). <br/>

possible reasons:
- 1. NO_C_FILE - There is no file in the user's folder with a .c extension, the score will be given, 0.
- 2. COMPILATION_ERROR – Compilation error (file does not compile). The score will be given, 10.
- 3. TIMEOUT – The compiled c file is running for more than 5 seconds. The grade to be given, 20.
- 4. WRONG – the output is different from the desired output. The grade to be given is 50.
- 5. SIMILAR - the output is different from the desired output but similar. The grade to be given, 75.
- 6. EXCELLENT - the output is the desired output. The grade to be given, 100

An example of the results.csv file content: <br/>
Monica, 100, EXCELLENT <br/>
Phoebe,0,NO_C_FILE <br/>
Rachel, 20, TIMEOUT <br/>
Ross,10,COMPILATION_ERROR <br/>
Joey, 50, WRONG <br/>
Chandler, 75, SIMILAR <br/>
Example of the configuration file content: <br/>
/home/os2012/students <br/>
/home/os2012/io/input.txt <br/>
/home/os2012/io/output.txt <br/>
Example of input file content (line 2:) <br/>
1 <br/>
5 4 <br/>
4 <br/>
Example of output file content (line 3:) <br/>
Please enter an operation <br/>
Please enter two numbers <br/>
The sum is 9 <br/>
Please enter an operation <br/>
Bye <br/>
An example of running the runtime file of your program: <br/>
[os2021@localhost ~]$ ./a.out /home/os2012/conf.txt <br/>

