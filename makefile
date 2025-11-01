 Instruction_Set_Processor:  Instruction_Set_Processor.o
	g++ -o  Instruction_Set_Processor  Instruction_Set_Processor.o

 Instruction_Set_Processor.o:  Instruction_Set_Processor.cpp
	g++ -c  Instruction_Set_Processor.cpp

clean:
	rm *.o  Instruction_Set_Processor
