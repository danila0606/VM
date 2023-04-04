new (2)                      # create object with 2 fields and store reference in G0

li $g1, (12)                 # load value 12 into G1

setfield $g0, (0)            # set field 0 of object in G0 to value in G1

li $g1, (30)                 # load value 30 into G1

setfield $g0, (1)            # set field 1 of object in G0 to value in G1

getfield $g2, $g0, (0)       # get field 0 of object in G0 and store in G2

halt