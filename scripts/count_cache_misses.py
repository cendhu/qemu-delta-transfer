import os
import sys

if(len(sys.argv) < 3):
	print "Usage :\n  python count_cache_misses.py <cache_misses_bitmap file>  <cache_content_bitmap file>"
	exit(0)

miss_file = open(sys.argv[1])
content_file = open(sys.argv[2])

miss_bitmaps = []
content_bitmaps = []

num_cache_pages = -1
page_size = -1
ram_pages = -1
no_longs = -1
BITS_PER_LONG = -1
MASK = 0xffffffffffffffff  #with BITS_PER_LONG 1s. Default is 32 1's


def bitwise_or(bitmap1, bitmap2):
	retbitmap = map((lambda x,y : x | y), bitmap1, bitmap2)
	return retbitmap

def bitwise_and(bitmap1, bitmap2):
	retbitmap = map((lambda x,y : x & y), bitmap1, bitmap2)
	return retbitmap

def count_bits(n):
	return bin(n).count('1')

def sum_1s(bitmap):
	t = [count_bits(x) for x in bitmap]
	return sum(t)

def read_file(file, bitmaps):
	isfirstline = True
	for line in file:
		global num_cache_pages
		global page_size
		global ram_pages
		global no_longs
		global BITS_PER_LONG
		global MASK
		if(isfirstline):
			isfirstline = False
			params = [int(x) for x in line.split()]
			num_cache_pages = params[0]
			page_size = params[1]
			ram_pages = params[2]
			no_longs = params[3]
			BITS_PER_LONG = params[4]
			if(BITS_PER_LONG == 32):
				MASK = 0xffffffff
		else:
			ints = [(int(x) & MASK) for x in line.split()]
			bitmaps.append(ints)

read_file(miss_file, miss_bitmaps)
read_file(content_file, content_bitmaps)


#print "header", num_cache_pages, page_size, ram_pages, no_longs, BITS_PER_LONG

#find cumulative cache content bitmaps i.e bitmaps[i] telling if cache contained any page in or before iteration i
#Note overwrite the succesive bitmaps during this
for i in range(len(content_bitmaps)-1):
	content_bitmaps[i+1] = bitwise_or(content_bitmaps[i], content_bitmaps[i+1])


print "iter#", "#misses", "#yes", "#no"

for i in range(len(miss_bitmaps)):
	if i==0 :
		cumu_b = [0] * len(miss_bitmaps[i])   #in first iteration, cumulative bitmap is all 0s
	else:
		cumu_b = content_bitmaps[i-1]  #cumulative cache content bitmap upto iteration previous iteration

	miss_b = miss_bitmaps[i]	

	present_b = bitwise_and(miss_b, cumu_b)  #tells if a cache miss occured for a page and was present in some previous iteration

	total_misses = sum_1s(miss_b)
	num_yes = sum_1s(present_b)
	num_no = total_misses - num_yes
	print (i+1), total_misses, num_yes, num_no 


############  Rough ###############

# a = [1 ,3 ,5]
# b = [3, 4, 11]

# print bitwise_and(a,b)
# print sum_1s(bitwise_and(a,b))

# t = [2, 4, 7, 3]

# print sum_1s(t)
#print bin(63).count('1')

# content_bitmaps = [[2,3], [3,8], [6,3]]
# for i in range(len(content_bitmaps)-1):
# 	content_bitmaps[i+1] = bitwise_or(content_bitmaps[i], content_bitmaps[i+1])
