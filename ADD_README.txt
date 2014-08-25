LOG FILES
==========
Pages in cache: filename :  <VM-name>_cache_log (in current directory)
    Format of file:
    #cache_pages   page_size    //first line
    <Iteration #> C0 C1 .....   //all remaining lines follow this format. Each containing #cache_pages entries.

    where C0 is page # present in cache[0] and is -1 if that cache loc is empty.
    Note that it is virtual page number(addr/PAGE_SIZE) and not virtual page address

    There will be as many lines as there are rounds/iterations.

Cache misses : filename :  <VM-name>_cache_misses_log (in current directory)
    Format of file:
    #cache_pages   page_size    //first line
    M1 M2 ......  -1    //all remaining lines follow this format

    where M1 is miss 1 in current round, and so on. It is finally followed by -1 as
    number of cache misses in a round is variable.

    There will be (#iterations + 1) lines in log file. One extra line because of the cache 
    misses happening in final ram_save_complete() call.
    
Dirty bitmap : filename : <VM-name>_dirty_bitmap_log (in current directory)
    This is already being printed (in given code).
    Format of file:
    #pages_in_ram    no_longs    BITS_PER_LONG  //no_longs is (#pages in ram/BITS_PER_LONG)
    N1 N2 ....    //all remaining lines follow this format
    
    each line contains dirty bitmap in chunks of BIT_PER_LONG bits (each Ni represented by unsigned int.     'no_longs' such entries in each line)

    There will be #iterations dirty bitmap lines
