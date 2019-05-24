import hashlib
import sys

def getMD5sum():
	BLOCKSIZE = 65536
	hasher = hashlib.md5()
	with open(sys.argv[1], 'rb') as afile:
	    buf = afile.read(BLOCKSIZE)
	    while len(buf) > 0:
	        hasher.update(buf)
	        buf = afile.read(BLOCKSIZE)
	hash_output = open('hash_source.key', 'w')
	hash_output.write(hasher.hexdigest())
	hash_output.close()

getMD5sum()