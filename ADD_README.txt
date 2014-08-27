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


Dirty bitmap : filename : <VM-name>_dirty_bitmap_log (in current directory)
    Format of file:
    #cache_pages   page_size  #pages_in_ram    no_longs    BITS_PER_LONG  //no_longs is (#pages in ram/BITS_PER_LONG)
    N1 N2 ....    //all remaining lines follow this format
    
    each line contains dirty bitmap in chunks of BIT_PER_LONG bits (each Ni represented by unsigned int.     'no_longs' such entries in each line)

    Bit i is 1 if page i got dirtied during current iteration. It is 0 otherwise.

    There will be #iterations dirty bitmap lines

