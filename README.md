# emlib

*Emlib is mainly used for the base library of embedded APP development.*

*Enjoy it!*

## Emlib include modules

### OS:
1. [*OK*]Abstract Lock Mechanism.
2. [*OK*]Posix Thread Task Manage.
3. [*OK*]Posix Condition Variable
4. [*OK*]Posix Semaphore(Based of Memory, and shared between of Threads)
5. I/O
6. Socket
7. DBMS 
8. [*OK*]Memory Pool Manage 
9. [*OK*]OS Stack Check Mechanism.
10. [*OK*]OS Time System
11. [*OK*]Heap Timer Task
12. [*OK*]Posix Timer Task

### Data Struct 
1. [*OK*]Double Linked List.
2. [*OK*]String
3. Queue
4. Array
5. Hash
6. Graph
7. Tree

### Algorithm
1. Sort
2. Search
3. ...

### MISC:
1. [*OK*]Base Log Manage.
2. [*OK*]C Exception Mechanism.

## How to Build emlib
1. Download emlib
2. cd emlib

### make shared libs

1. mkdir build
2. cd build
3. cmake -DBUILD_SHARED_LIBS=ON ..

### install prefix
1. cmake -DCMAKE_INSTALL_PREFIX=~/work/OKMX6UL-C2/app/install ..




