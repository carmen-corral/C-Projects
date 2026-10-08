/*======================================================================
 * PROJECT: C-cret Codex
 *----------------------------------------------------------------------
 * AUTHOR: Carmen Corral
 * EMAIL: ccorral@wisc.edu
 * ADDITIONAL SOURCES: NONE
 * FILE: ccret.c
 * COURSE: COMP SCI 354 - Fall 2026
 * INSTRUCTOR: Dahl
 * COPYRIGHT: 2026, Dahl
 * Posting or sharing this file with anyone outside of course staff prohibited.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "ccret.h"

/*
 * Entry point for this C-cret Codex application.  This program can be used
 * to encode and decode plain text using a variety of different substitution
 * ciphers.
 *
 * This program takes 2 or 3 command line arguments (in addition to this
 * executable's name).  The first argument is an optional -d, which specifies
 * that the message should be decoded rather than encoded with the given key.
 * The next argument is the key that should be used for encoding or decoding.
 * And the last argument is the message.  This program will the print out
 * the resulting encoded/decoded message to standard out.
 *
 * Examples:
 *   ./ccret A abc                -> encoded: ZYX
 *   ./ccret -d A ZYX             -> decoded: ABC
 *   ./ccret cc "hello, world"    -> encoded: JGNNQ, YQTNF
 */
int main(int argc, char **argv)
{
    // TODO: Step 2.2.
    // Update this main method to initialize the following local variables
    // based on the provided command line arguments.  When the number of
    // command line arguments is not correct, display usage message and end
    // the program with error code 1.

    char *key = "KEY";
    int decode = 0; // 0: encode message, 1: decode message
    char *message = "abc";
    char output[4]; // fix length to hold copy of message

    ccret(key, decode, message, output);
    printf("%s: %s\n", !decode ? "encoded" : "decoded", output);

    return 0;
}

/*
 * This function does the heavy lifting for the C-cret Codex application.
 * Arugments:
 * key is a c-string reference to the key used for encoding/decoding
 * pDecode is either 0: encode message with key, otherwise message is decoded
 * message is a c-string to the message being encoded or decoded
 * output references the memory where the encoded/decoded string is written
 */
void ccret(char *key, int pDecode, char *message, char *output)
{
    // TODO: Step 2.6.
    // Polyalphabetic cipher implemented when key starts with 'p'.

    // create cipher specific mapping from plain letters to encoded ones
    char substitutionMap[LETTER_COUNT];
    createSubstitutionMap(key, substitutionMap);
    // when decoding a message, invert this map
    if (pDecode)
        invertMap(substitutionMap);

    // encode/decode each char in message, one at a time
    for (int i = 0; i < strlen(message) + 1; i++)
    {
        char c = message[i];
        // convert char to a uppercase letter index 0-LETTER_COUNT
        int letterIndex = toupper(c) - 'A';
        if (letterIndex >= 0 && letterIndex < LETTER_COUNT)
            // when that index is valid letter, use map for substitution
            c = substitutionMap[letterIndex] + 'A';
        output[i] = c;
    }
}

/*
 * Creates a substitution map that can be used for encoding messages using
 * a variety of different ciphers.  This function makes use of the helper
 * functions: createAtbashMap, createCaesarMap, and createMixedMap.  When
 * the first letter of a key is 'a') it creates the map for an Atbash cipher,
 * 'c') it creates the map for a Caesar cipher, otherwise) it creates a map
 * for a Mixed Alphabet cipher.
 */
void createSubstitutionMap(char *key, char *map)
{
}

/*
 * This function inverts a substitution map so that the result can be used for
 * decoding messages rather than encoding them.  For example, if A maped to X
 * in the input map, then X will map back to A in that map after calling this
 * function.
 */
void invertMap(char *map)
{
    char invertedMap[LETTER_COUNT];
    for (int i = 0; i < LETTER_COUNT; i++)
        invertedMap[(int)map[i]] = i;
    memcpy(map, invertedMap, LETTER_COUNT * sizeof(char));
}

// TODO: Step 2.3.
// Implement function that create Atbash Cipher here.
// maps each letter in the alphabet to it's reverse: a -> z, b -> y ..
void createAtbashMap(char *key, char *map)
{
}

// TODO: Step 2.4.
// Implement function that create Caesar Cipher here.
void createCaesarMap(char *key, char *map)
{
}

// TODO: Step 2.5.
// Implement function that create Mixed Alphabet Cipher here.
void createMixedMap(char *key, char *map)
{
}

// EOF -----------------------------------------------------------------
