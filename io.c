#include "io.h"
#include "word.h"
#include "code.h"
#include "endian.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

// buffer for syms
static uint8_t syms_buff[BLOCK] = { 0 };
static int syms_index = 0;
static int syms_end = BLOCK + 1;

// buffer for pairs
static uint8_t pairs_buff[BLOCK] = { 0 };
static int bit_index = 0;

// total counts for syms and bits
uint64_t total_syms = 0;
uint64_t total_bits = 0;

// Reads in bytes until all bytes specified are actually read
int read_bytes(int infile, uint8_t *buf, int to_read) {
    // init vars for bytes read
    int total_read = 0;
    int curr_read = 0;
    int new_to_read = to_read;
    // loop until end of file or read all of specified bytes
    while (((curr_read = read(infile, buf, new_to_read)) > 0) && (total_read != to_read)) {
        // update vars for bytes read
        total_read += curr_read;
        buf += total_read;
        new_to_read -= total_read;
    }
    return total_read;
}

// Reads in bytes until all bytes specified are actually written
int write_bytes(int outfile, uint8_t *buf, int to_write) {
    // init vars for bytes written
    int total_written = 0;
    int curr_written = 0;
    int new_to_write = to_write;
    // loop until end of file or written all of specified bytes
    while (
        ((curr_written = write(outfile, buf, new_to_write)) > 0) && (total_written != to_write)) {
        // update vars for bytes written
        total_written += curr_written;
        buf += total_written;
        new_to_write -= total_written;
    }
    return total_written;
}

// reads in sizeof(FileHeader) bytes from input file
void read_header(int infile, FileHeader *header) {
    // set bytes to_read as sizeof(FileHeader)
    int to_read = sizeof(header);
    // read in bytes to_read from infile into header buf w/ type casted header
    read_bytes(infile, (uint8_t *) header, to_read);
    // check endianness
    if (big_endian()) {
        // swap endianness for data in FileHeader
        header->magic = swap32(header->magic);
        header->protection = swap16(header->protection);
    }
    // add header bits to total
    total_bits += (to_read * 8);
}

// reads in sizeof(FileHeader) bytes from input file
void write_header(int outfile, FileHeader *header) {
    // check endianness
    if (big_endian()) {
        // swap endianness for data in FileHeader
        header->magic = swap32(header->magic);
        header->protection = swap16(header->protection);
    }
    // set bytes to_write as sizeof(FileHeader)
    int to_write = sizeof(header);
    // write bytes to_write to outfile from header buf w/ typed casted header
    write_bytes(outfile, (uint8_t *) header, to_write);
    // add header bits to total
    total_bits += (to_write * 8);
}

// reads in symbols in buffer
bool read_sym(int infile, uint8_t *sym) {
    // check if we're already reading a block
    if (syms_index > 0) {
        // set sym as current index in sym buffer
        *sym = syms_buff[syms_index];
        // update sym index
        syms_index += 1;
        // if we reach end of sym buffer, read new block
        if (syms_index == BLOCK) {
            // to read a new block, we have to reset syms index to 0
            syms_index = 0;
        }
    }
    // read a new block
    else {
        int new_block = read_bytes(infile, syms_buff, BLOCK);
        if (new_block < BLOCK) {
            syms_end = new_block + 1;
        }
        // read sym in new block
        *sym = syms_buff[syms_index];
        // update sym index
        syms_index += 1;
    }
    // check if there are still syms to read
    if (syms_end != syms_index) {
        total_syms += 1;
        return true;
    } else {
        return false;
    }
}

// write a pair to outfile (pair is buffered)
void write_pair(int outfile, uint16_t code, uint8_t sym, int bitlen) {
    // writing from LSB first, so check if big_endian
    if (big_endian()) {
        // swap endiannnes of code to LSB -> MSB
        swap16(code);
    }

    int code_bit = 0;
    // looping while there are bits in code left to write
    while (code_bit < bitlen) {
        // check if buffer is full
        if (bit_index == (BLOCK * 8)) {
            // flush the buffer
            flush_pairs(outfile);
        }
        // if bit in code is set
        if ((code & (1UL << (code_bit % 16))) >> (code_bit % 16)) {
            // set bit in buffer
            pairs_buff[bit_index / 8] |= (1UL << (bit_index % 8));
        }
        // inc bit count in code and bit index
        code_bit += 1;
        bit_index += 1;
    }

    int sym_bit = 0;
    // looping while there are bits in syms left to write
    while (sym_bit < 8) {
        // if buffer is full
        if (bit_index == (BLOCK * 8)) {
            // flush the buffer
            flush_pairs(outfile);
        }
        // if bit in sym is set
        if ((sym & (1UL << (sym_bit % 8))) >> (sym_bit % 8)) {
            // set bit in buffer
            pairs_buff[bit_index / 8] |= (1UL << (bit_index % 8));
        }

        // inc bit count in sym and bit index
        sym_bit += 1;
        bit_index += 1;
    }

    // inc total bits by bits in code + sym
    total_bits += (bitlen + 8);
}

// write out remaining pairs to the output file
void flush_pairs(int outfile) {
    // convert bits to bytes to_flush
    int to_flush;
    if (bit_index % 8 == 0) {
        to_flush = bit_index / 8;
    } else {
        to_flush = (bit_index / 8) + 1;
    }
    // flush the toilet (from index 0 to curr index)
    write_bytes(outfile, pairs_buff, to_flush);
    // reset pairs buffer
    memset(pairs_buff, 0, BLOCK);
    // reset bit index
    bit_index = 0;
}

// read a pair from the input file and pass into pointers
bool read_pair(int infile, uint16_t *code, uint8_t *sym, int bitlen) {
    // reset code and sym when reading in new pair
    memset(code, 0, sizeof(uint16_t));
    memset(sym, 0, sizeof(uint8_t));

    // looping while there are bits in code left to read
    int code_bit = 0;
    while (code_bit < bitlen) {
        // if all bits processed, read another block
        if (bit_index == 0) {
            read_bytes(infile, pairs_buff, BLOCK);
        }
        // set bit in code as corresponding bit in buff
        if ((pairs_buff[bit_index / 8] & (1UL << (bit_index % 8))) >> (bit_index % 8)) {
            *code |= (1 << (code_bit % 16));
        }

        // inc bit count in code and bit index
        code_bit += 1;
        bit_index += 1;

        // if we're at the end of the BLOCK, reset index
        if (bit_index == (BLOCK * 8)) {
            bit_index = 0;
        }
    }

    // looping while there are bits left in sym to read
    int sym_bit = 0;
    while (sym_bit < 8) {
        // if all bits processed, read another block
        if (bit_index == 0) {
            read_bytes(infile, pairs_buff, BLOCK);
        }
        // set bit in sym as corresponding bit in sym
        if ((pairs_buff[bit_index / 8] & (1UL << (bit_index % 8))) >> (bit_index % 8)) {
            *sym |= (1 << (sym_bit % 8));
        }

        // inc bit count in sym and bit index
        bit_index += 1;
        sym_bit += 1;
        // if we're at the end of the BLOCK, reset index
        if (bit_index == (BLOCK * 8)) {
            bit_index = 0;
        }
    }
    // inc total bits by bits in code + sym
    total_bits += (bitlen + 8);

    // check endiannness
    if (big_endian()) {
        // if code is in big endian, swap to little (LSB -> MSB)
        *code = swap16(*code);
    }

    // there are pairs left to read if read code != STOP_CODE
    if (*code != STOP_CODE) {
        return true;
    }
    return false;
}

// write a word to the output file
void write_word(int outfile, Word *w) {
    uint32_t word_index = 0;
    // looping through syms in word
    while (word_index < w->len) {
        // check if buffer is filled
        if (syms_index == BLOCK) {
            // flush buffer
            flush_words(outfile);
        }
        // place each symbol in word to buffer
        syms_buff[syms_index] = w->syms[word_index];
        // inc syms index within buffer and word after placing symbol
        syms_index += 1;
        word_index += 1;
    }
    // inc total syms by syms in word
    total_syms += w->len;
}

// flush the words in the toilet
void flush_words(int outfile) {
    // from index 0 to curr index, print out all syms in buff
    write_bytes(outfile, syms_buff, syms_index);
    memset(syms_buff, 0, BLOCK);
    syms_index = 0;
}
