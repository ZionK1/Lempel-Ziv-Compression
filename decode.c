#include "word.h"
#include "code.h"
#include "io.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define OPTIONS "hvi:o:"

// gets bit length by repeatedly shifting right till 0
static int get_bitlen(uint16_t x) {
    int bit_len = 0;
    while (x != 0) {
        x >>= 1;
        bit_len += 1;
    }
    return bit_len;
}

int main(int argc, char **argv) {
    // set vars for encode
    int infile = 0;
    int outfile = 1;
    bool verbose = false;

    int opt = 0;
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'h':
            fprintf(stderr,
                "SYNOPSIS\n"
                "   Decompresses files with the LZ78 decompression algorithm.\n"
                "   Used with files compressed with the corresponding encoder.\n\n"
                "USAGE\n"
                "   ./decode [-vh] [-i input] [-o output]\n\n"

                "OPTIONS\n"
                "   -v          Display decompression statistics\n"
                "   -i input    Specify input to decompress (stdin by default)\n"
                "   -o output   Specify output of decompressed input (stdout by default)\n"
                "   -h          Display program usage\n");
            return 0;
        case 'v': verbose = true; break;
        case 'i':
            infile = open(optarg, O_RDONLY);
            if (infile == -1) {
                close(infile);
                perror("Couldn't open input file!\n");
                exit(1);
            }
            break;
        case 'o':
            outfile = open(optarg, O_WRONLY | O_CREAT | O_TRUNC);
            if (outfile == -1) {
                close(outfile);
                perror("Couldn't open output file!\n");
                exit(1);
            }
            break;
        default:
            fprintf(stderr,
                "SYNOPSIS\n"
                "   Decompresses files with the LZ78 decompression algorithm.\n"
                "   Used with files compressed with the corresponding encoder.\n\n"
                "USAGE\n"
                "   ./decode [-vh] [-i input] [-o output]\n\n"
                "OPTIONS\n"
                "   -v          Display decompression statistics\n"
                "   -i input    Specify input to decompress (stdin by default)\n"
                "   -o output   Specify output of decompressed input (stdout by default)\n"
                "   -h          Display program usage\n");
            return 1;
        }
    }

    // file stats
    struct stat FileData;
    fstat(infile, &FileData);

    // read in file header
    FileHeader header;
    read_header(infile, &header);

    // verify magic number
    if (!(header.magic == MAGIC)) {
        close(infile);
        close(outfile);
        perror("Magic number does not match. Cannot continue with decompression.\n");
        exit(1);
    }

    // make permission for outfile match protection bits in fileheader
    fchmod(outfile, header.protection);

    // create a new word table
    WordTable *table = wt_create();
    uint16_t curr_code = 0;
    uint16_t next_code = START_CODE;
    uint8_t curr_sym = 0;

    // while there are pairs left to read
    while (read_pair(infile, &curr_code, &curr_sym, get_bitlen(next_code))) {
        //printf("curr code = %u, curr sym = %u, next code = %u\n", curr_code, curr_sym, next_code);
        // append read symbol to word noted by curr code and add result to table
        table[next_code] = word_append_sym(table[curr_code], curr_sym);
        // write word constructed above to outfile
        write_word(outfile, table[next_code]);
        // increment next code
        next_code += 1;
        // if we've reached max code, reset the wt
        if (next_code == MAX_CODE) {
            wt_reset(table);
            next_code = START_CODE;
        }
    }

    // flush buffered words
    flush_words(outfile);

    // delete wt
    wt_delete(table);

    // close files
    close(infile);
    close(outfile);

    // check for printing decompression stats
    if (verbose) {
        // convert compressed total bits to bytes
        uint64_t bytes = 0;
        if (total_bits % 8 == 0) {
            bytes = total_bits / 8;
        } else {
            bytes = (total_bits / 8) + 1;
        }
        fprintf(stderr, "Compressed file size: %" PRId64 " bytes\n", bytes);
        fprintf(stderr, "Uncompressed file size: %" PRId64 " bytes\n", total_syms);
        float space_saving = (100.0 * (1.0 - ((float) bytes / total_syms)));
        fprintf(stderr, "Space saving: %.2f%%\n", space_saving);
    }
    return 0;
}
