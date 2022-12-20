# SadVM (Serega Aznaur Danila VM)
register based vm

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
  
