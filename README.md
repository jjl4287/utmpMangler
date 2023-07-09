# utmpMangler

## Overview

`utmpMangler` is a command-line tool designed to coalesce and filter "utmp" files. It addresses the problem of disorganized utmp files by merging multiple files into a single organized file. Additionally, it provides filtering options based on criteria like date and username to extract specific login/logout records from the merged utmp file. This tool was initially developed to help a professor analyze student login activity on a server.

## How it Works

The `utmpMangler` tool takes as input one or more utmp files. It coalesces them into a single file, applying filters based on user-specified options such as date and username. The filtered records are then either displayed in human-readable form on the standard output or written to an output file in a non-readable form.

The program uses system programming features and standard C library functions to read, write, and manipulate the utmp files. It utilizes the `utmp.h` header file to access the necessary data structures and defines. The tool also utilizes functions from the `<time.h>`, `<fcntl.h>`, `<stdio.h>`, `<string.h>`, `<stdlib.h>`, `<unistd.h>`, and `<sys/stat.h>` headers to handle file I/O, memory allocation, and other operations.

## Compatibility

The `utmpMangler` program is designed to work on Unix-like systems that support the utmp file format and provide the required system headers and functions mentioned above. It has been tested and verified to work on most Unix/Linux based systems.

## Download, Compile, and Use

To use the `utmpMangler` tool, follow these steps:

1. Download the source code file `utmpMangler.c`.

2. Compile the code using a C compiler. For example, using `gcc`:

   ```shell
   gcc -o utmpMangler utmpMangler.c
   ```
3. Run the tool by providing the appropriate command-line options and input files. Here are some examples:
    - Coalesce multiple utmp files and display the filtered records on the standard output:
    ```shell
    ./utmpMangler -dMM/YYYY -uUSERNAME file1.utmp file2.utmp ...
    ```
    - Coalesce utmp files and write the filtered records to an output file:
    ```shell
    ./utmpMangler -dMM/YYYY -uUSERNAME -ooutput.utmp file1.utmp file2.utmp ...
    ```
    - If no input files are specified, the program reads from the default utmp file /var/log/wtmp.
4. The program will display or write the filtered records based on the specified options.

## Example Use Cases

1. Display login records for a specific user within a given date range:
    ```shell
    ./utmpMangler -d01/2023 -ujohndoe
    ```
This command coalesces utmp files, filters the records for January 2023, and displays login records for the user "johndoe".
2. Write login records for all users to an output file:
    ```shell
    ./utmpMangler -oall_users.utmp
    ```
This command coalesces utmp files and writes all login records to the file "all_users.utmp".
Display login records for all users in a specific month:
    ```shell
    ./utmpMangler -d06/2023
    ```
This command coalesces utmp files and displays login records for all users in June 2023.
## Conclusion

The `utmpMangler` tool provides a solution for coalescing and filtering disorganized utmp files. Its flexible filtering options allow users to extract specific login/logout records based on criteria such as date and username. By utilizing system programming features and standard C library functions, it offers a reliable and efficient way to organize and analyze login activity. Feel free to use and modify this tool to suit your own requirements.


