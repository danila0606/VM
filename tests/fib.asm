li $r0, (1000)
li $g9, (1)
li $g10, (1)
li $g8, (1) #answer
li $r1, (0) # 0 or 1

call fib
j exit

fib:
    mov $g1, $r0
    addi $g1, (-2)
    le $g1, $nul
    jzs exit

    le $r1, $nul
    jzs add_1
    j add_2

from_add:
    addi $r0, (-1)
    call fib

add_1:
    li $r1, (1)
    add $g9, $g10
    mov $g8, $g9
    j from_add

add_2:
    li $r1, (0)
    add $g10, $g9
    mov $g8, $g10
    j from_add

exit:
    printi $g8              # print the result
    printc '\n'
    halt
