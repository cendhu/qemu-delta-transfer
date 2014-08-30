LOG FILES
==========
Pages in cache: filename :  <VM-name>_cache_log (in current directory)
    Format of file:
    #cache_pages   page_size  #pages_in_ram    no_longs    BITS_PER_LONG  //no_longs is (#pages in ram/BITS_PER_LONG)
    N1 N2 ....    //all remaining lines follow this format

    each line contains cache content bitmap in chunks of BIT_PER_LONG bits(Ni is unsigned int)
    i.e whether a virtual page is in cache at the end of iteration or not.
    Bit i is 1 if page i is in cache. Otherwise 0.

    There will be #iterations cache content bitmap lines


Cache misses : filename :  <VM-name>_cache_misses_log (in current directory)
    Format of file:
    #cache_pages   page_size  #pages_in_ram    no_longs    BITS_PER_LONG  //no_longs is (#pages in ram/BITS_PER_LONG)
    N1 N2 ....    //all remaining lines follow this format

    each line contains cache misses bitmap in chunks of BIT_PER_LONG bits(Ni is unsigned int)
    Bit i is 1 if cache miss occured for page i. Otherwise it is 0.

    There will be #iterations cache misses bitmap lines.

Cache Hits : filename : <VM-name>_cache_hits_log (in current directory)
    Format of file: same as Cache misses. Bit is 1 corresponding to cache hit. Otherwise it is 0.


Dirty bitmap : filename : <VM-name>_dirty_bitmap_log (in current directory)
    Format of file:
    #cache_pages   page_size  #pages_in_ram    no_longs    BITS_PER_LONG  //no_longs is (#pages in ram/BITS_PER_LONG)
    N1 N2 ....    //all remaining lines follow this format
    
    each line contains dirty bitmap in chunks of BIT_PER_LONG bits (each Ni represented by unsigned int.     'no_longs' such entries in each line)

    Bit i is 1 if page i got dirtied during current iteration. It is 0 otherwise.

    There will be #iterations dirty bitmap lines



SCRIPTS:

count_cache_misses.py: used with miss_bitmap.txt & cache_bitmap.txt 
=====================
    What it does? : If cache miss event occurred for a page 'n' during iteration i, then check
    whether the page 'n' was present in any of the previous iterations' cache
    content. For each iteration, print #misses, #yes, #no
    "yes" mean the page was present in any of the previous iterations.

    Usage : 
    python count_cache_misses.py <miss_bitmap file>  <cache_bitmap file>

    Sample Output:
        iter# #misses #yes #no
        1 0 0 0
        2 49922 0 49922
        3 34553 6425 28128
        4 26369 8681 17688
        5 25656 9999 15657
        6 24807 10485 14322
        7 35331 14239 21092
        8 23740 11106 12634
        9 18224 8759 9465
        10 17173 7999 9174
        11 18034 8656 9378
        12 20428 9846 10582
        13 18791 9627 9164
        14 16238 8332 7906
        15 26760 11917 14843
        16 34135 12700 21435
        17 668 211 457