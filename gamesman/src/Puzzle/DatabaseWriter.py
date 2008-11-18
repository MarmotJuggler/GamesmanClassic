import sys
import DatabaseHelper
import Solver
import Puzzle
import pickle
import time

def setList(lis, element, value, default):
	length = len(lis)
	if element < length:
		lis[element] = value
	else:
		for i in xrange(length,element):
			lis.append(default)
		lis.append(value)
	

def numBits(x):
	a = 0
	while x:
		x >>= 1
		a += 1
	return a

def solveDatabase(puzzname, options):
	if puzzname.find('/') != -1 or puzzname.find('\\') != -1:
		raise ArgumentException, "invalid puzzle"

	module = __import__(puzzname)
	puzzleclass = getattr(module, puzzname)
	
	print "Solving "+puzzleclass.__name__+" with options "+repr(options)+"..."
	puzzle = puzzleclass(**options) # instantiates a new puzzle object
	solver = Solver.Solver()
	solver.solve(puzzle,verbose=True)
	
	print "Solved!  Generating array of values"
	
	return solver

def writeDatabase(puzzname, options, solver):
	fields =[('remoteness',numBits(solver.get_max_level()))]
	bitClass = DatabaseHelper.makeBitClass(fields)
	
	CHUNKBITS=8
	CHUNK = 1<<CHUNKBITS
	
	filename = DatabaseHelper.getFileName(puzzname, options)
	print "writing to file %s"%filename
	f = open(filename, "wb")
	pickle.dump({'puzzle': puzzname,
		'options': options,
		'fields': fields,
		'chunkbits': CHUNKBITS}, f)
	chunkbase = (f.tell() + (CHUNK-1)) >> CHUNKBITS
	
	default = str(bitClass(remoteness=0)) # remoteness==0 but not solution means not seen.
	
	maxchunks = 1 + solver.maxHash/CHUNK
	print "Maximum hash is %d, number of %d-byte chunks to write is %d"%(solver.maxHash, CHUNK, maxchunks)
	
	lasttime = starttime = time.time()
	for chunknum in xrange(0,maxchunks):
		arr = []
		for chunkoff in xrange(CHUNK):
			position = (chunknum<<CHUNKBITS)+chunkoff
			myval = solver.seen.get(position, None)
			if not myval:
				continue
			
			val = bitClass(remoteness=myval)
			setList(arr, chunkoff, str(val), default)
		
		nexttime = time.time()
		pct = 100.0*float(1+chunknum)/maxchunks
		if chunknum==0:
			print "Writing first chunk out of %d to file %s"%(maxchunks,filename)
		elif nexttime - lasttime > 0.5 or chunknum==1:
			timeleft = (maxchunks-chunknum) * ((nexttime-starttime)/chunknum)
			minleft = int(timeleft/60)
			secleft = int(timeleft)%60
			print "[%3.2f%%] Writing chunk %d/%d to file %s, %d:%d minutes left"%(pct, chunknum, maxchunks, filename, minleft, secleft)
			lasttime = nexttime
		if arr:
			f.seek((chunkbase + chunknum)<<CHUNKBITS)
			f.write(''.join(arr))
	
	f.close()
	print "Done!"

if __name__=='__main__':
	#writeDatabase('Wheel',
	#	{'seqSize': 6})
	if len(sys.argv)<2:
		print >>sys.stderr, "Arguments: Python file to import (without .py)"
		sys.exit(0)
	solver = solveDatabase(sys.argv[1], {})
 	writeDatabase(sys.argv[1], {}, solver)
