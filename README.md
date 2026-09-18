# Project made with C
![brain-made label](imgs/88x31-light.png)

Board games and c libraries for arena allocators, dynamic arrays, dynamic hash tables and linked-list based queues. The board games and queue library were written in portuguese.

The arena library uses the mmap/mprotect POSIX syscalls to reserve 256Mb of contiguous virtual memory without allocating physical memory pages unless necessary. Support for Windows is possible but i didn't get around to doing it 😛. See arena.h for more information. 

The chess program implements checks, checkmates, promotions, en passant and castling.

The go program implements most of the japanese style rules, however i didn't implement seki.

![xadrez.c demo](imgs/xadrez.gif)
![go.c demo](imgs/go.gif)
![bingo.c](imgs/bingo.gif)
