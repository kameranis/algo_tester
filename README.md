# Simple Tester

For many problems a programmer may have some instances that have a unique solution
in a form of input-output pairs.
This tester aims to receive an executable, run it against said pairs and print out statistics

## Installation

Compile the source code:
```
make
```
Install (system-wide to `/usr/local/bin`) with sudo:
```
sudo make install
```
User-specific Installation (no sudo required)
```
make install INSTALL_PREFIX=~/.local/bin
```

## Options

-d directory
The directory in which the testcases are stored. Default is "tests".
In the directory the names should be input(whatever)-output(whatever).
For example if there is a file named input_huge.txt, its output should
be in output_huge.txt.

-n number
Number of testcases. The last testcases tend to be larger
and you might want to avoid them while writing your program.

-t seconds
Seconds the tester will wait for each testcase. If the program run for
at that time, it will be terminated and an appropriate message will be displayed

## Known Issues

Because the program compares your output to the desired character by
character, you should make sure you have the same line changes.
On windows it is "\r\n" while on Unix it is plain "\n".
In \*nix use:
```
sed -i 's/\r//g' test_directory/output*
```
