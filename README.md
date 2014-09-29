Monte Carlo Pi
==============

A Simple Multithreaded **Pi** estimation based on the **Monte Carlo** method written in *C++*.

![Example results](/examples/Monte%20Carlo%20Pi%20Accuracy.png?raw=true)  
*Example generated on a x64 Intel running Mac OS X with pseudo-random uniform real distribution.*

Requirements
------------
 * C++ 11

Compilation and running
-----------------------
```
g++ -std=c++11 main.cpp -o main
./main
```

Usage
-----
Just run the program and after that pass any STDIN to terminate.  
(Press any key if input is from terminal)

Outputing CSV data to file
---------------------------
The program also outputs CSV friendly data about deviation from a provided Pi aproximation to estimate how accurate the method is.

This parses the CSV data from the program output.
```/main | grep ".*,.*" > output.csv ```  
_Hint: Press enter to quit._

You might want to redirect the output of the program prior to saving the CSV output to keep the programs's output
```
./main > output
grep ".*,.*" output > output.csv
```

I hope you find it helpful. I tried to document it as much as possible.

License
-------
This source is distributed under The MIT License (MIT).
