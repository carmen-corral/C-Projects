/*======================================================================
 * FILE: ccret.h
 * PROJECT: C-cret Codex
 * COURSE: COMP SCI 354 - Fall 2026
 * INSTRUCTOR: Dahl
 * COPYRIGHT: 2026, Dahl
 * Posting or sharing this file with anyone outside of course staff prohibited.
 *----------------------------------------------------------------------
 * DO NOT MAKE ANY CHANGES TO THIS FILE.
 *----------------------------------------------------------------------
 */

#ifndef _CCRET_H_
#define _CCRET_H_

#define LETTER_COUNT 26

void ccret(char *key, int pDecode, char *message, char *output);
void createSubstitutionMap(char *key, char *map);
void invertMap(char *map);
void createAtbashMap(char *key, char *map);
void createCaesarMap(char *key, char *map);
void createMixedMap(char *key, char *map);

#endif // _CCRET_H_
