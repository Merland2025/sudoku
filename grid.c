#include <grid.h>
#include <colors.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Interal structure (hiden from outside) to represent a sudoku grid */
struct _grid_t
{
  size_t size;
  colors_t **cells;
};

struct choice_t
{
  size_t row;
  size_t column;
  colors_t color;
};

void fill_row(size_t size, FILE *f, char *row)
{
  if (!f || !row || !grid_check_size(size))
    return;

  size_t i = 0;
  char c = getc(f);
  row[i] = '\0';

  while(c != '\n') 
    switch(c)
      {
        case ' ':
        case '\t':
            c = getc(f);
          break;

        case '#':
            while ( c != '\n' && !feof(f))
              c = getc(f);
          break;

        case EOF:
            c = '\n';
          break;

        default:
            if (c != '\n' && i < size + 1)
            {
              row[i] = c;
              i = i + 1;
              c = getc(f);
            }
      }

  row[i] = '\0';
}

bool check_row(char *row, char *who)
{
  char list_total[66];
  size_t size = strlen(row);
  char *s = calloc(2, sizeof(char));
  if (!s)
    return false;
  
  strcpy(list_total, color_table);
  list_total[size] = EMPTY_CELL;
  list_total[size + 1] = '\0';

  for (size_t i = 0; i < size; i = i + 1)
  {
    s[0] = row[i];
    s[1] = '\0';
    if (!strspn(s,list_total))
    {
      *who = row[i];
      free(s);

      return false;
    }
  }
  free(s);

  return true; 
}

long push_row(grid_t *grid, char *row, size_t nb_row)
{
  size_t size = grid_get_size(grid);

  if (!size || !row)
    return -1;

  if (row[0] == '\0')
    return(nb_row);

  if (nb_row == size)
    return(nb_row + 1);

  for (size_t i = 0; i < size; i = i + 1)
    grid_set_cell(grid, nb_row, i, row[i]);
  
  return(nb_row +1);  
}

grid_t *grid_alloc(size_t size)
{
  grid_t *grid = NULL;

  if (!grid_check_size(size))
    return NULL;

  grid = calloc(1, sizeof(grid_t));
  if (!grid)
    return NULL;

  grid->cells = calloc(size, sizeof(colors_t *));
  if (!grid || !grid->cells)
    return NULL;
  
  for (size_t i = 0; i < size; i = i + 1)
  {
    grid->cells[i] = calloc(size, sizeof(colors_t));
    if (!grid->cells[i])
      return NULL;
  }

  grid->size = size;

  return grid;

}

void grid_free(grid_t *grid)
{
  size_t size = grid_get_size(grid);

  if (!size)
    return;

  for (size_t i = 0; i < size; i = i + 1)
    free(grid->cells[i]);
  
  free(grid->cells);
  free(grid);
}

size_t grid_get_size(const grid_t *grid)
{
  if (!grid || !grid_check_size(grid->size))
    return 0;

  return grid->size;
}

void grid_print(const grid_t *grid, FILE *fd)
{

  char *str_color = NULL;
  size_t size = grid_get_size(grid);
  
  if (!fd || !size)
    return;

  for (size_t i = 0; i < size; i = i + 1)
  {
    for(size_t j = 0; j < size; j = j + 1)
    {
      str_color = grid_get_cell(grid, i, j);
      if(!str_color)
        return;

      if (strlen(str_color) == size && size > 1)
        fprintf(fd,"%c ", EMPTY_CELL);
      else
        fprintf(fd,"%s ", str_color);

      free(str_color);
    }

    fprintf(fd,"%c",'\n');
  }
}

bool grid_check_char(const grid_t *grid, const char c)
{

  char list_total[66];
  size_t size = grid_get_size(grid);
  char s[2] = {c , '\0'};
  
  if (!size)
    return false;

  strcpy(list_total, color_table);
  list_total[size] = EMPTY_CELL;
  list_total[size + 1] = '\0';

  return  strspn(s,list_total);
}

bool grid_check_size(const size_t size)
{
  return size == 1 || size == 4 || size == 9 || size ==16 || size == 25 ||
      size == 36 || size == 49 || size == 64;
}

grid_t *grid_copy(const grid_t *grid)
{
  size_t size = grid_get_size(grid);

  if (!size)
    return NULL;

  grid_t *grid_cp = grid_alloc(size);
  for (size_t i = 0; i < size; i = i + 1)
  {
    for(size_t j = 0; j < size; j = j + 1)
      grid_cp->cells[i][j] = grid->cells[i][j];
  }
  return grid_cp;
}

char *grid_get_cell(const grid_t *grid, const size_t row, const size_t column)
{

  colors_t color_less = colors_empty();
  colors_t colors_box = colors_empty(); 
  size_t nb_colors = 0;
  size_t size = grid_get_size(grid);
  char *str_color = NULL;
  size_t color_index = 0;

  if (!size || row >= size || column >= size)
    return NULL;

  colors_box = grid->cells[row][column]; 
  nb_colors = colors_count(colors_box);
  if (!nb_colors)
    return NULL;

  str_color = calloc(nb_colors + 1, sizeof(char));
  if (!str_color)
    return NULL;

  for (size_t i = 0; i < nb_colors; i = i + 1)
  {
    color_index = 0;
    color_less = colors_rightmost(colors_box);
    colors_box = colors_xor(colors_box, color_less);

    while(color_less >>= 1)
      color_index = color_index + 1;
    
    str_color[i] = color_table[color_index];
  }
  str_color[nb_colors] = '\0';

  return str_color;
}

void grid_set_cell(grid_t *grid, const size_t row, const size_t column,
                   const char color)
{
  size_t size = grid_get_size(grid);
  size_t index_color = 0;
  colors_t colors_pool = colors_empty();
  
  if (!size || row >= size || column >= size || !grid_check_char(grid, color)) 
    return;

  if (color == EMPTY_CELL)
    colors_pool = colors_full(size);
  else 
  {
    while (color_table[index_color] != color)
      index_color = index_color + 1;

    colors_pool = colors_set(index_color);
  }

  grid->cells[row][column] = colors_pool;
}

static size_t grid_size_sqrt(const grid_t *grid)
{
  size_t size = grid_get_size(grid);
  size_t i = 0;
  size_t sum = 0;

  if (!size)
    return 0;

  while (sum != size)
  {
    sum = sum + 2*i + 1;
    i = i + 1;
  }

  return i;
}

static colors_t ****subgrid_alloc(const size_t size)
{
  colors_t ****subgrid = NULL;
  
  subgrid = calloc(NB_SUBGRID_TYPE, sizeof(colors_t ***));
  if (!subgrid)
    return NULL;

  for (size_t i = 0; i < NB_SUBGRID_TYPE; i = i + 1)
  {
    subgrid[i] = calloc(size, sizeof(colors_t **));
    if (!subgrid[i])
      return NULL;

    for (size_t j = 0; j < size; j = j + 1)
    {
      subgrid[i][j] = calloc(size, sizeof(colors_t *));
      if (!subgrid[i][j])
        return NULL;
    }
  }

  return subgrid;
}

static void subgrid_free(colors_t ****subgrid, const size_t size)
{
  for (size_t i = 0; i < NB_SUBGRID_TYPE; i = i + 1)
  {
    for (size_t j = 0; j < size; j = j + 1)
      free(subgrid[i][j]);

    free(subgrid[i]);
  }

  free(subgrid);
}

static colors_t ****get_grid_subgrid(const grid_t *grid)
{
  colors_t ****subgrid = NULL;
  size_t size = grid_get_size(grid);

  if (!size)
    return NULL;

  subgrid = subgrid_alloc(size);
  if (!subgrid)
    return NULL;

/* get column subgrids */
  for (size_t i = 0; i < size; i = i + 1)
    for (size_t j = 0; j < size; j = j + 1)
      subgrid[COL][i][j] = &grid->cells[j][i];

/* get row row subgrids */
  for (size_t i = 0; i < size; i = i + 1)
    for (size_t j = 0; j < size; j = j + 1)
      subgrid[ROW][i][j] = &grid->cells[i][j];

/* get block subgrids */
  size_t where = 0;
  size_t col_start = 0;
  size_t row_start= 0;
  colors_t sqrt = grid_size_sqrt(grid);

  if (!sqrt)
  {
    subgrid_free(subgrid, size);

    return NULL;
  }

  for (size_t i = 0; i < size; i = i + 1)
  {
    where = 0;
    col_start = (i % sqrt) * sqrt;
    row_start = i - (i % sqrt);
    for (size_t j = 0; j < sqrt; j = j + 1)
      for (size_t k = 0; k < sqrt; k = k + 1)
      {
        subgrid[BLOCK][i][where] = &grid->cells[row_start + j][col_start + k];
        where = where + 1;
      }

  }

  return subgrid;
}

bool grid_is_solved(grid_t *grid)
{
  size_t size = grid_get_size(grid);
  if (!size)
    return false;

  for (size_t i = 0; i < size; i = i + 1)
    for (size_t j = 0; j < size; j = j + 1)
      if (!colors_is_singleton(grid->cells[i][j]))
        return false;

  return true;
}

bool grid_is_consistent(grid_t *grid)
{
  bool consistency = true;
  size_t size = grid_get_size(grid);
  colors_t ****subgrid = get_grid_subgrid(grid);
  if (!subgrid)
    return false;

  for (size_t i = 0; i < NB_SUBGRID_TYPE; i = i + 1)
    for (size_t j = 0; j < size; j = j + 1)
      consistency = consistency && subgrid_consistency(subgrid[i][j], size);

  subgrid_free(subgrid, size);

  return consistency;
}

size_t grid_heuristics(grid_t *grid)
{
  bool alteration = true;
  size_t size = grid_get_size(grid);
  colors_t ****subgrid = get_grid_subgrid(grid);
  if (!subgrid)
    return 2;

  while(alteration)
  {
    alteration = false;

    for (size_t i = 0; i < NB_SUBGRID_TYPE; i = i + 1)
      for (size_t j = 0; j < size; j = j + 1)
        alteration = alteration || subgrid_heuristics(subgrid[i][j], size);
  }

  subgrid_free(subgrid, size);

  if (!grid_is_consistent(grid))
    return 2;

  if (!grid_is_solved(grid))
    return 0;

  return 1;
}

void grid_choice_free(choice_t *choice)
{
  free(choice);
}

bool grid_choice_is_empty(const choice_t *choice)
{
  return colors_is_equal(colors_empty(), choice->color);
}

void grid_choice_apply(grid_t *grid, const choice_t *choice)
{
  size_t size = grid_get_size(grid);
  if (!size)
    return;

  if (choice->row >= size || choice->column >= size)
    return;

  grid->cells[choice->row][choice->column] = choice->color;   
}

void grid_choice_blank(grid_t *grid, const choice_t *choice)
{
  size_t size = grid_get_size(grid);
  if (!size)
    return;

  if (choice->row >= size || choice->column >= size)
    return;

  grid->cells[choice->row][choice->column] = colors_full(size);   
}

void grid_choice_discard(grid_t *grid, const choice_t *choice)
{
  size_t c = choice->column;
  size_t r = choice->row;
  size_t size = grid_get_size(grid);
  if (!size)
    return;

  if (r >= size || c >= size)
    return;

  grid->cells[r][c] = colors_subtract(grid->cells[r][c], choice->color);   
}

void grid_choice_print(const choice_t *choice, FILE *fd)
{
  if (!fd || !choice)
    return;

  size_t color_index = 0;
  colors_t color = choice->color;

  while(color >>= 1)
    color_index = color_index + 1;
    
  fprintf(fd,"Next choice at grid[%ld][%ld] is %c\n",choice->row, choice->column, color_table[color_index]);
}

choice_t *grid_choice(grid_t *grid)
{
  size_t size = grid_get_size(grid);
  if (!size)
    return NULL;
  
  choice_t *choice = NULL;
  choice = calloc(1, sizeof(choice_t));
  size_t min_number_color = size + 1;
  size_t nb_colors = 0;
  colors_t choix = 0;

  for (size_t i = 0; i < size; i = i + 1)
    for (size_t j = 0; j < size; j = j + 1)
    {
      choix = grid->cells[i][j];
      if (!colors_is_singleton(choix))
      {
        nb_colors = colors_count(choix);
        if (nb_colors <= min_number_color)
          min_number_color = nb_colors;

      }
    }

  if (min_number_color > size)
    return NULL;

  for (size_t i = 0; i < size; i = i + 1)
    for (size_t j = 0; j < size; j = j + 1)
    {
      choix = grid->cells[i][j];
      nb_colors = colors_count(choix);
      if (nb_colors == min_number_color && grid_choice_is_empty(choice))
      {
        choice->row = i;
        choice->column = j;
        choice->color = colors_rightmost(choix);
      }

    }

  return choice;
  
}
