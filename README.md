# Assignment 6: Lempel-Ziv Compression

## Description:
	In this assignment, we implement two programs called encode and decode which perform LZ78
	compression and decompression, respectively. The core idea behind both of these compression 
	algorithms is to represent repeated patterns in data with pairs using pairs which are each
	comprised of a code and symbol. Both encode and decode can compress/decompress any file, text
	or binary. They operate on both little and big endian systems, use variable bit-length codes,
	and perform read and writes in efficient blocks of 4KB.
	
## Build:

To build the program:

```
$ make
```

## Running:

To run the encode program:

```
$ ./encode [OPTIONS]
```

```
OPTIONS:
    -h              Display program help and usage.
    -v              Display verbose program output.
    -i input        Specify input to compress (stdin by default).
    -o output       Specify output of compressed input (stdout by default).
```

To run the decode program:

```
$ ./decode [OPTIONS]
```

```
OPTIONS:
    -h              Display program help and usage.
    -v              Display verbose program output.
    -i input        Specify input to decompress (stdin by default).
    -o output       Specify output of decompressed input (stdout by default).
```

## Cleaning:

To clean the program files:

```
$ make clean
```

## Formatting:

To format the program files:

```
$ make format
```

## Files:

### Makefile
```
This contains all the instructions for the compiler to build the program.
```

### encode.c
```
This contains the implementation and main() functions for the encode program.
```

### decode.c
```
This contains the implementation and main() functions for the decode program.
```

### trie.c
```
This is the source file for the Trie ADT.
```

### trie.h
```
This is the header file for the Trie ADT.
```

### word.c
```
This is the source file for the Word ADT.
```

### word.h
```
This is the header file for the Word ADT.
```

### io.c
```
This is the source file for the I/O module.
```

### io.h
```
This is the header file for the I/O module.
```

### endian.h
```
This is the header file for the endianness module.
```

### code.h
```
This is the header file containing macros for reserved codes.
```

### DESIGN.pdf
```
This document contains the design of the program and describes the design
for the program.
```

### WRITEUP.pdf
```
This document contains all my findings and discoveries throughout the completion
of the assignment.
```
