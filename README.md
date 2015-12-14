<h1>Simple Tester</h1>

For many problems a programmer may have some instances that have a unique solution
in a form of input-output pairs. 
This tester aims to receive an executable, run it against said pairs and print out statistics

<h3>Instalation</h3>
```
git clone https://github.com/kameranis/algo_tester.git
make
```
<h3>Options</h3>

-d directory<br>
The directory in which the testcases are stored. Default is "tests".
In the directory the names should be input(whatever)-output(whatever).
For example if there is a file named input_huge.txt, its output should
be in output_huge.txt.

-n number<br>
Number of testcases. The last testcases tend to be larger
and you might want to avoid them while writing your program.

-t seconds<br>
Seconds the tester will wait for each testcase. If the program run for
at that time, it will be terminated and an appropriate message will be displayed

<h3>Issues</h3>

Because the program compares your output to the desired character by
character, you should make sure you have the same line changes.
On windows it is "\r\n" while on Unix it is plain "\n".<br>
In \*nix use:
```
sed -i 's/\r//g' test_directory/output*
```

