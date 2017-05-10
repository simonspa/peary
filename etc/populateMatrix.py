#!/usr/bin/python
matrix_file = open("matrix.cfg","w");
threshold = 15
countingmode = 0
testpulse = 0
longcnt = 1

matrix_file.write("#Peary CLICpix2 Matrix configuration\n")
matrix_file.write("# ROWL COL mask threshold countingmode testpulse longcnt\n")
for row in range(0,128):
    for column in range(0, 128):
        mask = 0
        # if row > 10 or column > 10:
        #     mask = 1
        matrix_file.write("%i %i %i %i %i %i %i\n" % (row, column, mask, threshold, countingmode, testpulse, longcnt))

matrix_file.close()
