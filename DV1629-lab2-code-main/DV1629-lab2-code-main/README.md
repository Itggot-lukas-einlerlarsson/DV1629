# DV1629-lab1-code
The code repository for lab 1 course (DV1629)

To compile and use the following tasks, run the following commands (used in UBUNTU environment):
Page replacement algorithm with First in First Out (FIFO) method
```
gcc -O2 -o fifo fifo.c && ./fifo "NO.PHYS.PAGES" "PAGE SIZE" "FILE NAME"
```
Page replacement algorithm with Least Recently Used (LRU) method
```
gcc -O2 -o lru lru.c && ./lru "NO.PHYS.PAGES" "PAGE SIZE" "FILE NAME"
```
Page replacement algorithm with Optimal method
```
gcc -O2 -o optimal optimal.c && ./optimal "NO.PHYS.PAGES" "PAGE SIZE" "FILE NAME"
```
