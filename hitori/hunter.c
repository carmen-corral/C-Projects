/*======================================================================
 * PROJECT: Hitori Hunter
 *----------------------------------------------------------------------
 * AUTHOR: Carmen Corral
 * EMAIL: ccorral@wisc.edu
 * ADDITIONAL SOURCES: <list any persons or resources (outside of those
 * provided by the course staff) that were used to complete this code here>
 * FILE: hunter.c
 * COURSE: COMP SCI 354 - Fall 2026
 * INSTRUCTOR: Dahl
 * COPYRIGHT: 2026, Dahl
 * Posting or sharing this file with anyone outside of course staff prohibited.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "hunter.h"

/*
 * Entry point for this Hitori Hunter application.  This program can be used to
 * solve hitori puzzles that are encoded as plain text files.
 *
 * This program takes 1 command line arguments specifying the filename of the
 * puzzle that you would like this program to solve.
 */
int main(int argc, char **argv)
{
    // use command line arguments to load board from plain text file
    if (argc != 2)
        error("Hitori Hunter usage: hunter PUZZLEFILE\n");
    char *filename = argv[1];
    int **puzzle;
    int size = loadPuzzle(filename, &puzzle);

    // transfer loaded board into a 1D array of Cells
    Cell solution[size * size];
    transferGrid(puzzle, solution, size);

    // make as much progress as possible toward solving this puzzle
    solvePuzzle(solution, size);

    // output progress toward solution to <file>.sln (length: + ".sln" + '\0')
    char solutionFilename[strlen(filename) + 4 + 1];
    strcpy(solutionFilename, filename);
    strcat(solutionFilename, ".sln");
    saveSolution(solution, size, solutionFilename);

    return 0;
}

/*
 * Helper function to more concisely report error messages through stderr
 * before ending the program with a return status of 1 (rather than 0).
 *
 * Arguments: Similar to printf, you can pass this method a format string
 * followed by a list of values to fill that string's placeholders.
 */
void error(char *format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stderr, "ERROR: ");
    vfprintf(stderr, format, args);
    exit(1);
}

/*
 * Loads Hitori puzzle from plain text file, and stores its contents into
 * a heap-allocated 2d int array.
 *
 * The first line of this plain text file must contains a single number
 * defining the size of the square puzzle (between 1 and MAX_BOARD_SIZE).
 * Each subsequent line contains one row of comma separate numbers (as
 * many as are dedfined by the previously read size).
 */
int loadPuzzle(char *filename, int ***puzzle)
{
    FILE *fin = fopen(filename, "r");
    if (fin == NULL)
        error("Unable to open file for reading: %s\n", filename);

    // reads board size
    char buffer[MAX_LINE_WIDTH];
    if (fgets(buffer, MAX_LINE_WIDTH, fin) == NULL)
        error("Unable to read first line of file: %s\n", filename);
    int size = atoi(buffer);
    if (size < 1 || size > MAX_BOARD_SIZE)
        error("Unable to read valid board size from file: %s\n", buffer);

    // allocate memory for entire puzzle grid
    int **grid = malloc(sizeof(int *) * size);
    if (grid == NULL)
        error("Unable to allocate memory for grid rows.\n");
    for (int i = 0; i < size; i++)
    {
        grid[i] = malloc(sizeof(int) * size);
        if (grid[i] == NULL)
            error("Unable to allocate memory for grid columns.\n");
    }

    // read board contents
    int row = 0;
    while (fgets(buffer, MAX_LINE_WIDTH, fin) != NULL && row < size)
    {
        char *token;
        int col = 0;
        for (token = strtok(buffer, ","); token != NULL; token = strtok(NULL, ","))
        {
            grid[row][col] = atoi(token);
            if (grid[row][col] < 1 || grid[row][col] > MAX_BOARD_SIZE)
                error("Unable to read column number: %s\n", token);
            col++;
        }
        if (col != size)
            error("Not enough columns in row: %s\n", buffer);
        row++;
    }
    if (row != size)
        error("Not enough rows in board: %i/%i\n", row, size);

    fclose(fin);
    *puzzle = grid;
    return size;
}

/*
 * Transfers 2D heap-allocated int array into a 1D stack-allocated Cell
 * array.  The heap memory is then freed, after it has been copied..
 */
void transferGrid(int **puzzle, Cell *solution, int size)
{
    // store the board integers (1-31) as individual bytes within a 1D array,
    // organized in row major order
    int index = 0;

    for (int r = 0; r < size; r++)
    {
        for (int c = 0; c < size; c++)
        {
            // need to copy the contents of puzzle into solution
            solution[index++] = puzzle[r][c];
        }
    }

    // free all dynamically allcoated memory that was used by puzzle
    for (int i = 0; i < size; i++)
        free(puzzle[i]);
    free(puzzle);
}

/*
 * Saves solution progress to the specified file.  Circled cells are saved
 * as the circled number, blacked out cells are saved as #'s, and all other
 * cells are saved as ?'s.
 */
void saveSolution(Cell *solution, int size, char *filename)
{
    FILE *fout = fopen(filename, "w");
    if (fout == NULL)
        error("Unable to open file for writing: %s\n", filename);

    fprintf(fout, "%i\n", size);
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            // Update the following line to store the contents of the Cell
            // in row = i and column = j into the variable value below.
            Cell value = solution[i * size + j];

            int isCircle = value & CIRCLE;
            int isBlackedOut = value & BLACK;
            int cellNumber = NUMBER(value);
            if (isCircle)
                fprintf(fout, "%i", cellNumber);
            else if (isBlackedOut)
                fprintf(fout, "#");
            else
                fprintf(fout, "?");
            if (j < size - 1)
                fprintf(fout, ",");
        }
        fprintf(fout, "\n");
    }
    fclose(fout);
}

/*
 * This method uses the most significant bits to encode whether a cell has been blacked
 * out or not. The method checks if the cell has been previously blacked out or circled,
 * and either returns or prints an error message.
 */
void blackout(Cell *solution, int size, int row, int column)
{
    // cell we want to potentially black out
    int index = row * size + column;

    // if the cell has previously been blacked out, return
    if (solution[index] & BLACK)
        return;

    // if the cell has previously been circled, error() w/ error code 1
    if (solution[index] & CIRCLE)
        error("Cannot black out cell because it was previously circled.\n");

    // marking the cell as black
    solution[index] = solution[index] | BLACK;

    // mark neighbors (hor & vert) as circled, making a circle deduciton
    // ^^ should call the circle() function for each neighboring cell
    // above
    if (row - 1 >= 0)
        circle(solution, size, row - 1, column);
    // below
    if (row + 1 < size)
        circle(solution, size, row + 1, column);
    // left
    if (column - 1 >= 0)
        circle(solution, size, row, column - 1);
    // right
    if (column + 1 < size)
        circle(solution, size, row, column + 1);
}

/*
 * This method uses the second most significant bit to encode whether a cell has been
 * circled or not. The method checks if the cell has been previously circled or blacked
 * out, and either returns or prints an error message.
 */
void circle(Cell *solution, int size, int row, int column)
{
    // cell we want to potentially circle
    int index = row * size + column;

    // if the cell was previously circled, return
    if (solution[index] & CIRCLE)
        return;

    // if the cell was previously blacked out, error() w/ error code 1
    if (solution[index] & BLACK)
        error("Cannot circle cell because it was previously blacked out.\n");

    // marking the cell as circled
    solution[index] = solution[index] | CIRCLE;

    // get number at index
    int num = NUMBER(solution[index]);

    // iterate through the other cells in the same row AND column,
    // any non-black cell that contains the same number must be blacked out.
    // ^^ should call blackout() function
    // row
    for (int r = 0; r < size; r++)
    {
        // if r is the row that index is in, ignore (already circled)
        if (r == row)
            continue;

        // else if it isn't, baclk it out
        int indexOfCellToBlack = r * size + column;

        // mark it as blacked out
        if (!(solution[index] & BLACK) && NUMBER(solution[indexOfCellToBlack]) == num)
            blackout(solution, size, r, column);
    }
    // col
    for (int c = 0; c < size; c++)
    {
        // if c is the column that index is in, ignore (already circled)
        if (c == column)
            continue;

        // else if it isn't, black it out
        int indexOfCellToBlack = row * size + c;

        // mark it as blacked out
        if (!(solution[index] & BLACK) && NUMBER(solution[indexOfCellToBlack]) == num)
            blackout(solution, size, row, c);
    }
}

void solvePuzzle(Cell *solution, int size)
{
    // Update this function to call makeSandwichDeductions, and
    // makeDoubletDeductions
    makeSandwichDeductions(solution, size);
    makeDoubletDeductions(solution, size);
}

/*
 * This function searches for cells that are sandwiched between neighbors containing the same
 * value (either left and right, or above and below).
 */
void makeSandwichDeductions(Cell *solution, int size)
{
    // any number that has two identical numbers on opposite sides of itself cannot be
    // black, because one of the two identical numbers must be black, and it cannot
    // be adjacent to another black cell.
    for (int r = 0; r < size; r++)
    {
        for (int c = 0; c < size; c++)
        {
            // check if the top and bottom of the cell are the same
            int top = (r - 1) * size + c;
            int bottom = (r + 1) * size + c;

            if (NUMBER(solution[top]) == NUMBER(solution[bottom]))
                circle(solution, size, r, c);

            // check if the right and left of the cell are the same
            int right = r * size + (c + 1);
            int left = r * size + (c - 1);

            if (NUMBER(solution[right]) == NUMBER(solution[left]))
                circle(solution, size, r, c);
        }
    }
}

/*
 * This function searches the entire board for occurrences of two neighboring cells that
 * contain the same number. Whenever a pair is found, this function should call blackout
 * for every other cell in that same line (either row or col) that contain the same number.
 */
void makeDoubletDeductions(Cell *solution, int size)
{
    // in the case of two identical, adjacent numbers, if another cell occurs in the
    // same row or col containing the same number, the latter cell must be black.
    // otherwise, if it remains non-black, this would result in either two cells with
    // the same number in the same row or col, or two adjacent black cells, neither of
    // which are allowed.
    for (int r = 0; r < size; r++)
    {
        for (int c = 0; c < size; c++)
        {
            // check if cells next to each other are the same (row)
            int index = r * size + c;
            int nextIndex = (r + 1) * size + c;

            // if to make sure they do not got past size
            if (r + 1 < size)
            {
                // if they are equal, blackout every other cell with that same num
                if (NUMBER(solution[index]) == NUMBER(solution[nextIndex]))
                {
                    // black out other cells in row
                    for (int i = 0; i < size; i++)
                    {
                        // do not black out the index
                        if (i == r || i == r + 1)
                            continue;

                        int cellToBlack = i * size + c;

                        if (NUMBER(solution[cellToBlack]) == NUMBER(solution[index]))
                            blackout(solution, size, i, c);
                    }
                }
            }

            // check if cells next to each other are the same (col)
            int colIndex = r * size + c;
            int nextColIndex = r * size + (c + 1);

            // if to make sure they go not go past size
            if (c + 1 < size)
            {
                // if they are equal, blackout every other cell with the same num
                if (NUMBER(solution[index]) == NUMBER(solution[nextColIndex]))
                {
                    // black out other cells in col
                    for (int i = 0; i < size; i++)
                    {
                        // do not black out the index
                        if (i == c || i == c + 1)
                            continue;

                        int cellToBlack = r * size + i;

                        if (NUMBER(solution[cellToBlack]) == NUMBER(solution[colIndex]))
                            blackout(solution, size, r, i);
                    }
                }
            }
        }
    }
}

// EOF -----------------------------------------------------------------