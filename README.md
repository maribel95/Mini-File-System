# Mini-File-System
The practice implements a file system (based on Unix ext2) on a virtual disk (file), the file management module of the operating system and the interaction of user processes.

The file system is hierarchical, with a tree structure of directories and files, but you can also create physical links, which will actually turn it into a graph.
The file system is indexed allocation (128-byte inodes with 12 direct pointers, 1 single indirect pointer, 1 double indirect pointer and 1 triple indirect pointer).
Each file (that is, each inode) contains the minimum meta-information necessary for the primitives.
The file system is prepared to work with a block size between 512 bytes and 4,096 bytes (it must be a constant in the program to be able to easily change the value if desired)
The name of the file that acts as the virtual device and that the file system will store is dynamic; that is, it is not defined in the source programs.
The allocation of space to files is dynamic: new blocks are assigned as they are needed.
Concurrency control methods are implemented in libraries to access file system management metadata (superblock, bitmap, and inode array): critical sections must be defined and semaphores used where appropriate.
Within the libraries, the concurrent access of several processes to the data in the same file is not controlled, since that is the responsibility of the applications.
