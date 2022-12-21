li $r0, (10)
li $r1, (1)                 # prepare $r1

call fact
j exit

fact:
    le $r0, $nul
    jzs fact_return
    mul $r1, $r0
    addi $r0, (-1)
    call fact
    
fact_return:
    ret

exit:
    printi $r1              # print the result
    printc '\n'
    halt
