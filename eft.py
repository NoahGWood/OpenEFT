#!/usr/bin/python3
import sys
from core.core import Core

def main(fname=None):
	if not fname:
		# Get user input
		fname = input("Input File: ")
		if len(fname) < 1:
			fname = "test.png"
	# Create the core processor
	c = Core(ifile=fname)
	# Create EFT object
	c.GenEFT()

	
if __name__ in '__main__':
	x = len(sys.argv)
	if(x==1):
		main()
	else:
		main(sys.argv[1])
