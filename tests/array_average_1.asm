.text
    li $g0, test_i32

    li $g1, (13)
    sw $g1, $g0, (0)
    li $g1, (2)
    sw $g1, $g0, (4)
    li $g1, (9)
    sw $g1, $g0, (8)
    li $g1, (4)
    sw $g1, $g0, (12)
    li $g1, (8)
    sw $g1, $g0, (16)
    li $g1, (6)
    sw $g1, $g0, (20)

    li $r0, (0)                 # prepare $r0
    li $g2, (6)                 # array length
    muli $g2, (4)               # sizeof(int32)

middle:
    mov $g5, $g2                # save array's size in $g5
    le $g2, $nul                # jump to return if
    jzs middle_return           # we are at the array's beginning
    
    addi $g2, (-4)              # $g2 <- array index (shifts by sizeof(int32) to the left)
    mov $g3, $g0                # $g3 = array beginning
    add $g3, $g2                # shift $g3 pointer by the current index
    lw $g4, $g3, $nul           # load to $g4 from the array cell we need

    printi $g4
    printc ' '
    add $r0, $g4               # compute the sum in $r0
    j middle

middle_return:
    printc '\n'
    li $g6, sum_equals
    prints $g6
    printi $r0

    divi $r0, (6)               # divide by the number of elements

    printc '\n'
   # li $g6, average_equals
   # prints $g6
    printi $r0
    

    printc '\n'
    halt

.data

.asciiz sum_equals: "sum = "
.asciiz average_equals: "average = "


.i32 test_i32: (20)
