#!/usr/bin/env python
import sys

XOR_MIN = 0x78
XOR_INC = 0x2
XOR_MAX = 0x90

def encode(infile, outfile):
	xor_value = XOR_MIN
	b = bytearray(open(infile, 'rb').read())
	for i in range(len(b)):
		b[i] ^= xor_value
		xor_value += XOR_INC
		if xor_value > XOR_MAX:
			xor_value = XOR_MIN
	open(outfile, 'wb').write(b)

def usage():
	print "Usage: " + sys.argv[0] + " <input file> <output file>"

if __name__ == '__main__':
	argc = len(sys.argv)
	if argc != 3:
		usage()
	else:
		encode(sys.argv[1], sys.argv[2])
	sys.exit(0)
