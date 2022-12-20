li $g0, (5)
li $r0, (1)                 # prepare $r0

call fact
j exit

fact:
    le $g0, $nul
    jzs fact_return
    mul $r0, $g0
    addi $g0, (-1)
    call fact
    
fact_return:
    ret

exit:
    printi $r0              # print the result
    printc '\n'
    halt
