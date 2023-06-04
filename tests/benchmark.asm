.text

    # main
    # $f0, $f1, $f2, $f3 - args of function
    li $f0, (N)
    li $f1, (M)
    call foo
    halt

    ctor0:
        sfd $f0, $r1, (0) # set field 0 in object, which $r1 points to by val in register $f0 
        ret

    ctor1:
        sfd $f0, $r1, (0) # set field 0 in object, which $r1 points to by val in register $f0
        sfd $nul, $r1, (1) # set field 1 in object, which $r1 points to by zero pointer 
        ret

    m1_0:
        gfd $r0, $r1, (1) # get field 1 in object, which $r1 points to, store it in $r0
        ret

    m1_1:
        sfd $f0, $r1, (1) # set field 1 in object, which $r1 points to by val in $f0 
        ret

    dump:
        mov $g0, $nul # i
    dump_cycle:
        li $g7, (M)
        ge $g0, $g7 # check if i >= M
        jzs dump_ret

        # get object with number in $g0 from objects array in $f0 and store pointer to it in $r1
        goa $f0, $g0
        eq $r1, $nul # check if $r1 points to null
        jzs dump_print_null_0

        clm (0) # call 0th method of object, which $r1 points to. Result saves in $r0
        eq $r0, $nul # check if $r0 points to null
        jzs dump_print_null_1

        gfd $g2, $r0, (0) # get field 0 of object, which $r0 points to and store it in $g2
        printi $g2
        addi $g0, (1) # i += 1
        j dump_cycle:

    dump_print_null_0:
        li $g10, str1
        prints $g10
        addi $g0, (1) # i += 1
        j dump_cycle:
    
    dump_print_null_1:
        li $g10, str1
        prints $g10
        addi $g0, (1) # i += 1
        j dump_cycle:

    dump_ret:
        ret



    foo:
        noba $f1, $g1, (1) # new array of objects, type - class 1, count - value in $f1, result saves in $g1

        li $g0, (1) # i
    foo_cycle:
        li $g7, (N)  
        gt $g0, $g7 # check if i > N
        jzs foo_ret

        mov $f0, $g0 # arg of 1_ctor
        nob $g2, (1) # new object, type - class 1, result saves in $g2 and $r1 
        mov $g3, $g0
        li $g4, (3)
        rem $g3, $g4 # g3 = $g3 % $g4
        eq $g3, $nul # check if (i % 3 == 0)
        jzs first_if

    continue_while_1:
        mov $f0, $g0 # arg of 0_ctor
        nob $g5, (0) # new object, type - class 0, result saves in $g5 and $r1
        mov $g3, $g0
        li $g4, (5)
        rem $g3, $g4 # g3 = $g3 % $g4
        eq $g3, $nul # check if (i % 5 == 0)
        jzs second_if
    
    continue_while_2:
        mov $g11, $g2 # outer
        addi $g0, (1) # i += 1
        j foo_cycle

    first_if:
        mov $g3, $g0
        li $g4, (M)
        rem $g3, $g4 # g3 = $g3 % $g4
        addi $g3, (-1) # g3 = i % M - 1
        mov $r1, $g1
        soa $g2, $g3 # store object which $g2 points to on addr $g3 in objects array - $r1
        j continue_while_1

    second_if:
        mov $r1, $g2
        mov $f0, $g5 # arg of 1st method of 1st class
        clm (1) # call 1st method of object, which $r1 points to
        j continue_while_2

    foo_ret:
        mov $f0, $g1 # f0 - arg of dump
        call dump
        ret

.data
.class {.i32} # 1 field - i32
.class {.i32 .0} # 2 fields - i32, class 0

.i32 N: (4000000)
.i32 M: (1000)
.i32 three: (3)
.i32 five: (5)
.asciiz str1: "Foo:null"
.asciiz str2: "Foo.Bar:null"

