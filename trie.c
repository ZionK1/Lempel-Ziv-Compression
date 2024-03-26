#include "trie.h"
#include "code.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// constructor for TrieNode
TrieNode *trie_node_create(uint16_t code) {
    // allocate memory for TrieNode ADT
    TrieNode *n = (TrieNode *) calloc(1, sizeof(TrieNode));
    // if successful, set code and children node pointers to NULL
    if (n) {
        n->code = code;
        for (int i = 0; i < ALPHABET; i += 1) {
            n->children[i] = NULL;
        }
    }
    return n;
}

// desctructor for TrieNode
void trie_node_delete(TrieNode *n) {
    if (n) {
        free(n);
    }
}

// initialize a trie
TrieNode *trie_create(void) {
    // initialize root TrieNode with EMPTY_CODE
    TrieNode *root = trie_node_create(EMPTY_CODE);
    // if successful, return root
    if (root) {
        return root;
    } // otherwise, false
    return NULL;
}

// reset a trie to just the root TrieNode
void trie_reset(TrieNode *root) {
    // if root exists
    if (root) {
        // looping through root's children (0 - 255)
        for (int i = 0; i < ALPHABET; i += 1) {
            // delete child node and all of it's children
            if (root->children[i]) {
                trie_delete(root->children[i]);
                root->children[i] = NULL;
            }
        }
    }
}

// delete a sub-trie starting from trie rooted at node n
void trie_delete(TrieNode *n) {
    // if n exists
    if (n) {
        // looping through n's children (0 - 255)
        for (int i = 0; i < ALPHABET; i += 1) {
            // if child exists at index i
            if (n->children[i]) {
                // recursive call to check if that child has more children
                trie_delete(n->children[i]);
            }
        }
        // once we hit node w/o children, delete node
        trie_node_delete(n);
        // set pointer to NULl
        n = NULL;
    }
}

// return a pointer to child node representing the sym, NULL if sym doesn't exist
TrieNode *trie_step(TrieNode *n, uint8_t sym) {
    // if child with index sym exists, pointer will return child node, otherwise NULL
    if (n->children[sym]) {
        return n->children[sym];
    } else {
        return NULL;
    }
}
