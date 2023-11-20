# Mini-File-System
The practice implements a file system (based on Unix ext2) on a virtual disk (file), the file management module of the operating system and the interaction of user processes.


<img width="625" alt="Captura de pantalla 2023-11-20 a las 8 45 41" src="https://github.com/maribel95/Mini-File-System/assets/61268027/e205888f-58e1-472b-8754-6de0b678ca18">


## Structure

The practice will be composed of the following elements:
- A set of libraries, divided into levels, that will provide the functionality to the primitives to access the file system and manage it.
- A mi_mkfs program to create, with the help of these libraries, the file system on the disk.
- The file system (SF) will actually be contained in a file (simulating a secondary memory device), which we will call disk and has the following structure:

  
  <img width="480" alt="Captura de pantalla 2023-11-20 a las 9 02 43" src="https://github.com/maribel95/Mini-File-System/assets/61268027/393eb76b-f0a6-4401-ac7c-d7cd0ec275fa">

- A set of programs to operate via console, including one that displays the contents of a file on the screen (mi_cat), one that changes its access permissions (mi_chmod), one that creates physical links (mi_ln), one that lists directories (mi_ls), one that deletes files and directories (mi_rm), and one that shows the meta information of a file (mi_stat).
  -  Its operation is similar to the basic operation of the corresponding GNU/Linux programs cat, chmod, ln, ls, rm/rmdir, stat.
- A simulator program in charge of creating test processes that concurrently access the file system (using a mutex semaphore) so that the correct functioning of the aforementioned libraries can be verified.


## Requirements
- The file system is hierarchical, with a tree structure of directories and files, but you can also create physical links, which will actually turn it into a graph.
  
- The file system is indexed allocation (128-byte inodes with 12 direct pointers, 1 single indirect pointer, 1 double indirect pointer and 1 triple indirect pointer).
  
- Each file (that is, each inode) contains the minimum meta-information necessary for the primitives.
  
- The file system is prepared to work with a block size between 512 bytes and 4,096 bytes (it must be a constant in the program to be able to easily change the value if desired)
  
- The name of the file that acts as the virtual device and that the file system will store is dynamic; that is, it is not defined in the source programs.
  
- The allocation of space to files is dynamic: new blocks are assigned as they are needed.
  
- Concurrency control methods are implemented in libraries to access file system management metadata (superblock, bitmap, and inode array): critical sections must be defined and semaphores used where appropriate.
  
- Within the libraries, the concurrent access of several processes to the data in the same file is not controlled, since that is the responsibility of the applications.

## OPERATIONS:

- **my_dir()**: Obtains the content of a directory in a string of characters (you can separate its content by ':') as well as the number of files listed.
- **mi_stat()**: Obtains the meta information of a file (that is, its inode).
- **mi_read()**: Reads a certain number of bytes from a file, from an initial position, puts them in a memory buffer and returns the total number of bytes read.
- **mi_write()**: Writes to a file, from an initial position, a certain number of bytes from a memory buffer and returns the total number of bytes written.
- **my_creat()**: Creates the specified file/directory.
- **my_link()**: Creates the link from one directory entry to the inode specified by another directory entry. Both entries must correspond to files.
- **mi_unlink()**: Deletes the specified physical link and, if it is the last existing one, deletes the file itself.
- **mi_chmod()**: Change the access permissions to the file.
- **mi_touch()**: Update the timestamps of an existing file or to create an empty file if it does not exist.
- **mi_rmdir()**: Removes empty directories.
- **mi_rm_r()**: Removes directories and their contents recursively.
