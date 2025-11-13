# ARM-Processor-Simulator

### Description
This program is a console-based application that simulates a simplified ARM-like processor. It supports register-based operations, memory access instructions, NZCV flag updates, and conditional execution. The simulator reads an input file containing assembly-style instructions, executes them sequentially, and prints the resulting register values, memory state, and processor flags. This project demonstrates instruction parsing, state management, arithmetic/logic operations, branching logic, and object-oriented design in C++.

### Prerequisites To Compile
- Windows 10/11  
- A C++ compiler such as **MinGW-w64 or g++(G++) 

### To Compile
Using g++ (MinGW-w64 or g++ itself
g++ -std=c++17 -O2 -o ProcessorSimulator.exe *.cpp


### To Run
Place your instruction input file in the same directory as the executable, then run:

ProcessorSimulator.exe instructions.txt

### To Clean Build Files
 remove files with rm on linux
rm ProcessorSimulator
rm *.o

And for windows, use delete!

del ProcessorSimulator.exe
del *.o

# License
This project is licensed under GPLv3. See `LICENSE` for details.

ARM cpu and instructions are owned by ARM Holdings plc.
