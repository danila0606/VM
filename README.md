# VM
register based vm

SadVM (Serega Aznaur Danila VM)
==================================================
build: 

1) Assembler: 
cd assembler
cd build
cmake ..
cmake --build .
./Assembler.exe ../../tests/array_average.asm

2) VM:
cd vm
cd build
cmake ..
cmake --build .
./SadVM.exe ../../tests/array_average.bin
  
