#ifndef GRID_H
#define GRID_H

#define EMPTY_CELL '_'
#define MAX_GRID_SIZE 64
#define NB_SUBGRID_TYPE 3

#define COL 0
#define ROW 1
#define BLOCK 2

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

static const char color_table[] = 
  "123456789" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "@" "abcdefghijklmnopqrstuvwxyz"
  "&*";

/* Sudoku grid (forward declaration to hide the implementation) */
typedef struct _grid_t grid_t;
typedef struct choice_t choice_t;

/* fill char in the given row from a given a given file */
void fill_row(size_t size, FILE *f, char *row);

/* check if the given row has only allowed char, if not the given char
   is remplaced by the first disallowed char found */
bool check_row(char *row, char *who);

/* If possible, translate the given row from char to colors_t. Then fill 
   the given grid at the given row index (i.e nb_row) with the content of 
   the translated row.
   return an updated row index */
long push_row(grid_t *grid, char *row, size_t nb_row);

/* memory allocation for a grid of a given size */
grid_t *grid_alloc(size_t size);

/* free the allocated memory of given grid */
void grid_free(grid_t *grid);

/* print the given grid on the given file descriptor */
void grid_print(const grid_t *grid, FILE *fd);

/* check if the given char is allowed in the given grid */
bool grid_check_char (const grid_t *grid, const char c);

/* check if the given size is acceptable i.e 1, 4, 9, 16, 25, 36, 49, 64 */
bool grid_check_size(const size_t size);

/* return a deep copy of a given grid */
grid_t *grid_copy(const grid_t *grid);

/* return the colors as a string of the given cell of the given grid */
char *grid_get_cell(const grid_t *grid, const size_t row, const size_t column);

/* return the size of a given grid. If the size is not allowed, return NULL */
size_t grid_get_size(const grid_t *grid);

/* from a given grid, set the cell at given coordinate to given char color as
   a colors_t */
void grid_set_cell(grid_t *grid, const size_t row, const size_t column,
                   const char color);

/* check if the grid has only singleton */
bool grid_is_solved(grid_t *grid);

/* check if all subgrid of a given grid are consistent */
bool grid_is_consistent(grid_t *grid);

/* apply heuristics to all subgrid of a given grid, and return a number 
   corresponding to the state of the grid afterward:
   - '0' if the grid is not solved
   - '1' if the grid is solved
   - '2' if the grid is not solved and inconsistent */
size_t grid_heuristics(grid_t *grid);



/* free the allocated memory of given choice */
void grid_choice_free(choice_t *choice);

/* check if the choice has a color equal to O */
bool grid_choice_is_empty(const choice_t *choice);

/* apply the given choice on the grid. The color and the coordinate are taken
   from within the choice */
void grid_choice_apply(grid_t *grid, const choice_t *choice);

/* set the color of a given grid, at the given choice's coordiate, to the max
   possible color */ 
void grid_choice_blank(grid_t *grid, const choice_t *choice);

/* discard the choice's color, at the given choice's coordinate, 
   in the given grid */
void grid_choice_discard(grid_t *grid, const choice_t *choice);

/* print the given choice's coordinate and color on the given file descriptor */
void grid_choice_print(const choice_t *choice, FILE *fd);

/* return a choice made by taking the coordinate and the rightmost color
   of the first cell with the least number of color in a given grid */
choice_t *grid_choice(grid_t *grid);

#endif /* GRID_H */
