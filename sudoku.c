#include <colors.h>
#include <err.h>
#include <getopt.h>
#include <grid.h>
#include <string.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "sudoku.h"

static bool verbose = false;

typedef enum { mode_first, mode_all} mode_t;

static grid_t *file_parser(char *filename)
{
  FILE *f = fopen(filename,"r");
  grid_t *grid = NULL;
  size_t nb_row = 0;
  char *first_row = NULL; 
  size_t size = 0;

  if (!f)
    goto open_file_pb;
  
  first_row = calloc(MAX_GRID_SIZE + 2, sizeof(char));
  if (!first_row)
    goto memory_allocation_pb;

  do
  {
    fill_row(MAX_GRID_SIZE, f, first_row);
  } while(!strlen(first_row) && !feof(f));

  size = strlen(first_row);

/* check if the size of the first line is acceptable */ 
  if (!grid_check_size(size)) 
    goto row_size_pb;

  grid = grid_alloc(size);
  if (!grid)
    goto memory_allocation_pb;

  nb_row = push_row(grid, first_row, nb_row); 

  char who = '?';
  if (!check_row(first_row, &who))
    goto wrong_char_pb;

  free(first_row);

  char *row = calloc((size + 2), sizeof(char));
  if (!row)
    goto memory_allocation_pb;

  while (!feof(f))
  {
    fill_row(size,f,row);

/* check if the row is at the correct size */
    if (strlen(row) != size && strlen(row) != 0)
      goto row_size_pb;

    nb_row = push_row(grid, row, nb_row);
    if (!check_row(row, &who))
      goto wrong_char_pb;
  }
  free(row);

/* check if the number of row is correct */
  if (nb_row != size)
    goto number_row_pb;

  fclose(f);

  return(grid);

  open_file_pb:
    errx(EXIT_FAILURE,"file %s could not be opened",filename);

  memory_allocation_pb:
  {
    if(!nb_row && !size)
      free(first_row);

    else
      grid_free(grid);
      
    fclose(f);

    errx(EXIT_FAILURE,"An error occured during memory allocation");
  }

  row_size_pb:
  {
    if (!nb_row)
    {
      free(first_row);
      fclose(f);

      errx(EXIT_FAILURE,"error invalid grid size");
    }
    else
    {
      free(row);
      grid_free(grid);
    }

    fclose(f);

    errx(EXIT_FAILURE,"line %ld is malformed!(wrong number of columns)\n",
         nb_row + 1);
  }

  wrong_char_pb:
  {
    if (nb_row == 1)
      free(first_row);
    else
      free(row);

    grid_free(grid);
    fclose(f);

    errx(EXIT_FAILURE,"wrong character '%c' at line %ld \n", who, nb_row);
  }

  number_row_pb:
  {
    if(nb_row  > size)
    {
      nb_row = nb_row - size;
      grid_free(grid);
      fclose(f);

      errx(EXIT_FAILURE,"grid has %ld excess line(s)\n",nb_row);
    }
    else
    {
      nb_row = size - nb_row;
      grid_free(grid);
      fclose(f);

      errx(EXIT_FAILURE,"grid has %ld missing line(s)",nb_row);
    }
  }
}

static grid_t *grid_solver(grid_t *grid, const mode_t mode, FILE *fd)
{
  if (!grid)
    return NULL;

  size_t grid_status = grid_heuristics(grid);
  printf("grid_status = %ld\n", grid_status);
  choice_t *choice = NULL;
  grid_t *grid_cp = NULL;
  grid_t *status_check = NULL;
  bool  is_a_solution = false;

  switch (grid_status)
    {
      case 0:
          grid_cp = grid_copy(grid);
          choice = grid_choice(grid_cp);
          if (!choice || !grid_cp)
            return NULL;

          grid_choice_apply(grid_cp, choice);
          grid_choice_free(choice);
          status_check = grid_solver(grid_cp, mode, fd);
          grid_free(grid_cp);

          if (!status_check)
          {
            choice = grid_choice(grid);
            if (!choice)
              return NULL;

            grid_choice_discard(grid, choice);
            grid_choice_free(choice);
            grid = grid_solver(grid, mode, fd);
          }

        break;

      case 1:
          if (mode == mode_all)
          {
            fprintf(fd, "\n");
            grid_print(grid, fd);
            return NULL;
          }
          else
          {
            is_a_solution = true;
            goto solution;
          }

      case 2:
          return NULL;
        break;
    }
  solution:
  {
    if (!is_a_solution)
    {
      printf("------STOP!------\n");
      return grid;
    }
    else
    {
      printf("------START------\n");
      grid_print(grid,fd);
      return grid;
    }

  }
}

int main (int argc, char **argv)
{

/* arguments parser */ 
  int grid_size = DEFAULT_SIZE;
  FILE *fd = stdout;
  bool solver = true; 
  bool unique = false;
  mode_t all = false;

  static struct option long_opts[] =
  {
    {"help", no_argument, NULL, 'h'},
    {"version", no_argument, NULL, 'V'},
    {"verbose", no_argument, NULL, 'v'},
    {"output", required_argument, NULL, 'o'},
    {"unique", no_argument, NULL, 'u'},
    {"generate", optional_argument, NULL, 'g'},
    {"all", no_argument, NULL, 'a'},
    {NULL, no_argument, NULL, 0}
  };

  int optc;

  while ((optc = getopt_long (argc, argv, "vuao:g::hV",long_opts, NULL)) != -1)
    switch (optc)                                                                 
      {                                                                           
      case 'h':
          fprintf(stdout, "Usage:\tsudoku [-a|-o FILE|-v|-V|-h] FILE ...\n"
            "\tsudoku -g[SIZE] [-u|-o FILE|-v|-V|-h]\n"
            "Solve or generate Sudoku grids of various sizes"
            " (1,4,9,16,25,36,49,64)\n\n"
            " -a,--all\t\tsearch for all possible solutions\n"
            " -g[N],--generate[=N]\tgenerate a grid of size NxN (default:9)\n"
            " -u,--unique\t\tgenerate a grid with unique solution\n"
            " -o FILE,--o FILE\twrite solution to FILE\n"
            " -v,--verbose\t\tverbose output\n"
            " -V,--version\t\tdisplay version and exit\n"
            " -h,--help\t\tdisplay this help and exit\n");
        return EXIT_SUCCESS;

      case 'V':
          fprintf(stdout,"sudoku %d.%d.%d\n"
            "Solve/generate sudoku grids"
            "(possible sizes: 1, 4, 9, 16, 25, 36, 49, 64)\n", VERSION,
            SUBVERSION, REVISION);
        return EXIT_SUCCESS;

      case 'v':
          verbose = true;
        break;

      case 'u':
          unique = true;
          if (solver)
          {
            unique = false;
            warnx("warning: option 'unique' conflict with solver mode, disabl"
                   "ing it!\n");

            unique = false;
          }
        break;

      case 'a':
          all = true;
          if (!solver)
          {
            warnx("warning: option 'all' conflict with generator mode, disabl"
                  "ing it!\n");

            all = false;
          }
        break;

      case 'g':
          if (optarg)
            grid_size = strtol(optarg, NULL, 10);

          if (!grid_check_size(grid_size))
            goto grid_size_pb;

          solver = false;
        break;

      case 'o':
          if (optarg)
          {
            if (fd != stdout)
              fclose(fd);

            fd = fopen(optarg,"w+");
            if (!fd)
              goto open_file_pb;

           }
        break;

      default:
          goto option_pb;
      }

/* solver mode */
  grid_t *grid = NULL;
  FILE *open_test = NULL;

/* check if a grid is provided */
  if (solver)
  {
    if (optind >= argc)
      goto no_input_pb;

    while (optind < argc)
    {
      open_test = fopen(argv[optind],"r");
      if (!open_test)
        goto open_file_pb;

      fclose(open_test);

/* grid parser */
      grid = file_parser(argv[optind]);
      grid_print(grid, fd);

/* grid solver */
      grid = grid_solver(grid, all, fd);
      fprintf(fd, "\n");
      grid_print(grid, fd);
      grid_free(grid);
      optind = optind + 1;
    }
  }
  if (fd != stdout)
    fclose(fd);

  return EXIT_SUCCESS;    

  open_file_pb:
    errx(EXIT_FAILURE,"file could not be opened\n");

  grid_size_pb:
    errx(EXIT_FAILURE, "error invalid grid size\n");

  option_pb:
    errx(EXIT_FAILURE, "error: invalid option '%s'!\n", argv[optind - 1]); 

  no_input_pb:
    errx(EXIT_FAILURE, "error: no input grid given\n");

}    
