#!/usr/bin/python
countingmode = 1
testpulse = 0
longcnt = 1

import sys

print 'Number of arguments:', len(sys.argv), 'arguments.'
print 'Argument List:', str(sys.argv)


c = sys.argv[1]
r = sys.argv[2]
threshold = sys.argv[3]

matrix_file = open("matrix_px" + str(c) + "_" + str(r) + "_cnt_trim" + str(threshold) + ".cfg","w");

matrix_file.write("#Peary CLICpix2 Matrix configuration\n")
matrix_file.write("# ROW COL mask threshold countingmode testpulse longcnt\n")
for row in range(0,128):
    for column in range(0, 128):
        mask = 1
        if row == int(r) and column == int(c):
            print "Unmasked: " + str(c) + "," + str(r)
            mask = 0
        matrix_file.write("%i %i %i %i %i %i %i\n" % (row, column, mask, int(threshold), countingmode, testpulse, longcnt))

matrix_file.close()
