.text

    printc 'a'

    li $g11, (10) # N
    noba $g11, $g1, (0) # new array of objects, type - class 0, count - value in $g11, result saves in $g1

    mov $g0, $nul # i

    foo_cycle:
        ge $g0, $g11 # check if i >= N
        jzs foo_ret

        mov $f0, $g0 # arg of 0_ctor
        nob $g2, (0) # new object, type - class 0, result saves in $g2 and $r1 
        mov $r1, $g1
        soa $g2, $g0 # store object, $g2[$g0] = $r1
        addi $g0, (1) # i += 1
        j foo_cycle

    foo_ret:
        mov $f0, $g1 # f0 - arg of dump
        call dump
        halt

dump:
        mov $g0, $nul # i
    dump_cycle:
        li $g11, (10) # N
        ge $g0, $g11 # check if i >= M
        jzs dump_ret

        # get object with number in $g0 from objects array in $f0 and store pointer to it in $r1
        goa $g0, $f0

        gfd $g2, $r1, (0) # get field 0 of object, which $r1 points to and store it in $g2
        printi $g2
        printc ' '
        addi $g0, (1) # i += 1
        j dump_cycle

    dump_ret:
        ret

ctor0:
    sfd $f0, $r1, (0)
    ret

ctor1:
    ret

m1_0:
    ret

.data

# class definitiones must be in begining 
.class {.i32} # 1 field - i32
.class {.i32 .0} # 2 fields - i32, class 0