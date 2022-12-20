li $g0, (7)
call fact
j exit

fact:
    li $r0, (1)             # $r0 = ans
    li $r1, (2)             # $r1 = one
    
while_cycle:
    lt $g0, $r1             # if g0 <= 1, jump to return
    jzs return              # jump if condition true
    mul $r0, $g0            # r0 *= g0
    addi $g0, (-1)          # g0 -= 1
    j while_cycle

return:
    ret

exit:
    printi $r0              # print the result
    printc '\n'
    halt
