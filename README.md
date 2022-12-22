# SadVM (Serega Aznaur Danila VM)
Register-based staticly-typed virtual machine<br />
==================================================<br />
Time measurement results:

Average of 30 array elements
![Screenshot](img/avg.jpg)

1000-th fibonacci number
![Screenshot](img/fib.jpg)
==================================================<br />
Build: <br />

1) Assembler: <br />
cd assembler  <br />
cd build<br />
cmake ..<br />
cmake --build .<br />
./Assembler.exe ../../tests/array_average.asm<br />

2) VM:<br />
cd vm<br />
cd build<br />
cmake ..<br />
cmake --build .<br />
./SadVM.exe ../../tests/array_average.bin<br />
  
