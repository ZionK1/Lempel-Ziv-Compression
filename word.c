#include "word.h"
#include "code.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// constructor for a word
Word *word_create(uint8_t *syms, uint32_t len) {
    // allocate mem for Word ADT
    Word *w = calloc(1, sizeof(Word));
    if (w) {
        // set len for Word
        w->len = len;
        // allocate mem for syms array in our Word
        if (syms) {
            w->syms = calloc(len, sizeof(uint8_t));
        } else {
            w->syms = NULL;
        }
        // if w->syms set
        if (w->syms) {
            // loop through syms to set passed in syms
            for (uint32_t i = 0; i < len; i += 1) {
                w->syms[i] = syms[i];
            }
        }
    }
    return w;
}

// construct a new word appended with passed in symbol
Word *word_append_sym(Word *w, uint8_t sym) {
    // construct new word to append symbol to with len of (w->len + 1)
    // so as to allocate mem for extra symbol
    Word *new_word = word_create(&sym, w->len + 1);
    // loop to set syms of original word to new word
    for (uint32_t i = 0; i < w->len; i += 1) {
        new_word->syms[i] = w->syms[i];
    }
    // append the passed in symbol
    new_word->syms[w->len] = sym;
    return new_word;
}

// destructor for word
void word_delete(Word *w) {
    // if memory allocated for Word ADT / Word isn't NULL
    if (w) {
        // if array for symbols in Word allocted
        if (w->syms) {
            // free symbols array and set pointer to NULL
            free(w->syms);
            w->syms = NULL;
        }
        // free Word ADT itself
        free(w);
    }
}

// create new WordTable, an array of Words
WordTable *wt_create(void) {
    // allocate memory for WordTable with size of MAX_CODE
    WordTable *wt = calloc(MAX_CODE, sizeof(WordTable));
    // initialize single Word at index EMPTY_CODE
    // this Word should be empty and have str len of 0
    wt[EMPTY_CODE] = word_create(NULL, 0);
    return wt;
}

// reset a WordTable to contain just the empty Word
void wt_reset(WordTable *wt) {
    // starting from START_CODE (2) to MAX_CODE, loop through the WordTable
    // and delete Words and set them to NULL, leaving the empty Word
    for (int i = START_CODE; i < MAX_CODE; i += 1) {
        word_delete(wt[i]);
        wt[i] = NULL;
    }
}

// Destructor for WordTable
void wt_delete(WordTable *wt) {
    if (wt) {
        // delete words in entire WordTable, from STOP_CODE to MAX_CODE
        for (int i = STOP_CODE; i < MAX_CODE; i += 1) {
            if (wt[i]) {
                word_delete(wt[i]);
                wt[i] = NULL;
            }
        }
        // free WordTable ADT itself
        free(wt);
        wt = NULL;
    }
}
