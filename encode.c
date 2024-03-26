#include "trie.h"
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
                "   Compresses files using the LZ78 compression algorithm.\n"
                "   Compressed files are decompressed with the corresponding decoder.\n\n"
                "USAGE\n"
                "   ./encode [-vh] [-i input] [-o output]\n\n"
                "OPTIONS\n"
                "   -v          Display compression statistics\n"
                "   -i input    Specify input to compress (stdin by default)\n"
                "   -o output   Specify output of compressed input (stdout by default)\n"
                "   -h          Display program help and usage\n");
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
                "   Compresses files using the LZ78 compression algorithm.\n"
                "   Compressed files are decompressed with the corresponding decoder.\n\n"
                "USAGE\n"
                "   ./encode [-vh] [-i input] [-o output]\n\n"
                "OPTIONS\n"
                "   -v          Display compression statistics\n"
                "   -i input    Specify input to compress (stdin by default)\n"
                "   -o output   Specify output of compressed input (stdout by default)\n"
                "   -h          Display program help and usage\n");
            return 1;
        }
    }
    // find file size and protection bit mask of infile
    struct stat FileData;
    fstat(infile, &FileData);
    //off_t filesize = FileData.st_size;
    mode_t prot = FileData.st_mode;

    // init header with prot bits
    FileHeader header = { 0 };
    header.magic = MAGIC;
    header.protection = prot;

    // make permission for outfile match protection bits in fileheader
    fchmod(outfile, header.protection);

    //write out file header to outfile
    write_header(outfile, &header);

    // trie stuff
    TrieNode *root = trie_create();
    TrieNode *curr_node = root;
    TrieNode *prev_node = NULL;
    uint16_t next_code = START_CODE;
    uint8_t prev_sym = 0;
    uint8_t curr_sym = 0;

    // while there are syms left to read
    while (read_sym(infile, &curr_sym)) {
        // set next node
        TrieNode *next_node = trie_step(curr_node, curr_sym);
        // we have seen the current prefix
        if (next_node) {
            // move on to next node
            prev_node = curr_node;
            curr_node = next_node;
        } else {
            // new prefix, write out pair with code of bit length next_code
            //printf("curr_node->code = %u, prev_sym = %u\n", curr_node->code, prev_sym);
            write_pair(outfile, curr_node->code, curr_sym, get_bitlen(next_code));
            // create new child node
            curr_node->children[curr_sym] = trie_node_create(next_code);
            // point back to root
            curr_node = root;
            // inc next available code
            next_code += 1;
        }
        // if we're at MAX_CODE
        if (next_code == MAX_CODE) {
            // reached MAX_CODE, reset code
            next_code = START_CODE;
            // reset trie to just root
            trie_reset(root);
            // curr node should point back to root
            curr_node = root;
        }
        // update prev sym as the curr sym
        prev_sym = curr_sym;
    }
    // check if we're at root node, if not continue matching prefix
    if (curr_node != root) {
        //printf("prev_node->code = %u, prev_sym = %u\n", prev_node->code, prev_sym);
        write_pair(outfile, prev_node->code, prev_sym, get_bitlen(next_code));
        next_code = (next_code + 1) % MAX_CODE;
    }

    // signal end of compression using STOP_CODE and bit_length of next_code
    //printf("prev_node->code = %u, prev_sym = %u\n", STOP_CODE, 0);
    write_pair(outfile, STOP_CODE, 0, get_bitlen(next_code));

    // flush any unwritten, buffered pairs
    flush_pairs(outfile);

    // delete trie
    trie_delete(root);

    // close files
    close(infile);
    close(outfile);

    // check for printing compression stats
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
        float space_saving = (100.0 * (1.0 - ((float) bytes / (float) total_syms)));
        fprintf(stderr, "Space saving: %.2f%%\n", space_saving);
    }
    return 0;
}
