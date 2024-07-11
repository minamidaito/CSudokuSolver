/*****************************************************************************/
/* Add #define DEBUG for debug build.                                        */
/*****************************************************************************/

/*****************************************************************************/
/* Header files.                                                             */
/*****************************************************************************/
#include <string.h>
#include <stdio.h>
#include <assert.h>

/*****************************************************************************/
/* Handy constants.                                                          */
/*****************************************************************************/
#define SIZE 9
#define BLOCK_SIZE 3
#define TRUE 1
#define FALSE 0

/*****************************************************************************/
/* Assert macro.                                                             */
/*****************************************************************************/
#ifdef DEBUG
#define ASSERT(X) assert(X)
#else
#define ASSERT(X)
#endif

/*****************************************************************************/
/* Prototypes.                                                               */
/*****************************************************************************/
static int initialize(FILE *);
static int attempt_to_solve(void);
static int number_known(void);
static int do_check(void);
static void proc_block(int, int);
static int unique_value(int, int);
static void dump(void);
static void proc_row(int, int);
static void proc_col(int, int);

/*****************************************************************************/
/* Global variables for tracking recursion depth.                            */
/*****************************************************************************/
static int max_rec_depth = 0;
static int cur_rec_depth = 0;

/*****************************************************************************/
/* Global solution array.                                                    */
/*                                                                           */
/* The second-to-top entry is used to keep track of the number of            */
/* candidates.  This is redundant information but is an optimization trading */
/* off storage for speed.                                                    */
/*                                                                           */
/* We also trade off storage for speed by storing the unique candidate, once */
/* it's found, in the top array entry.                                       */
/*****************************************************************************/
static int sudoku[SIZE][SIZE][SIZE + 2];

/*****************************************************************************/
/* Handy macros.                                                             */
/*****************************************************************************/
#define FIXED_VAL(X,Y) sudoku[X][Y][SIZE+1]
#define CAND_COUNT(X,Y) sudoku[X][Y][SIZE]

int main (int argc, char *argv[])
{
  int rc;
  FILE *file;

  if (argc != 2)
  {
    /*************************************************************************/
    /* Print usage information.                                              */
    /*************************************************************************/
    printf("Use: sudoku input.txt (input.txt in same form as sample1.txt)\n");
    rc = FALSE;
    goto EXIT;
  }

  /***************************************************************************/
  /* Attempt to open the input file.                                         */
  /***************************************************************************/
  file = fopen(argv[1], "r");

  if (file == NULL)
  {
    printf("Failed to open input file %s\n", argv[1]);
    rc = FALSE;
    goto EXIT;
  }

  /***************************************************************************/
  /* Initialize our data structures and read in input data.                  */
  /***************************************************************************/
  rc = initialize(file);

  if (rc == FALSE)
  {
    printf("Initialization failed.\n");
    goto EXIT;
  }

  /***************************************************************************/
  /* Close input file.                                                       */
  /***************************************************************************/
  (void)fclose(file);

  /***************************************************************************/
  /* Show the user what went in.                                             */
  /***************************************************************************/
  printf("\nInput:\n");
  dump();

  /***************************************************************************/
  /* Attempt to solve the puzzle.  This is a recursive routine.              */
  /***************************************************************************/
  rc = attempt_to_solve();

  if (rc == FALSE)
  {
    printf("Couldn't solve it.\n");
  }
  else
  {
    /*************************************************************************/
    /* Solved it.  Dump out the solution.                                    */
    /*************************************************************************/
    printf("Output:\n");
    dump();
    printf("Maximum recursion depth was %d\n", max_rec_depth);
  }

EXIT:

  return (rc);
}

static int initialize(FILE *file)
{
  int ii;
  int jj;
  int kk;
  int xx;
  int rc = TRUE;

  /***************************************************************************/
  /* Clear out the working array.  Wind through all the entries.             */
  /***************************************************************************/
  for (ii=0;ii<SIZE;ii++)
  {
    for (jj=0;jj<SIZE;jj++)
    {
      /***********************************************************************/
      /* Initialize next entry.  First make every number a candidate.        */
      /***********************************************************************/
      for (kk=0;kk<SIZE;kk++)
      {
        sudoku[ii][jj][kk] = TRUE;
      }

      /***********************************************************************/
      /* Initialize the candidate count.                                     */
      /***********************************************************************/
      CAND_COUNT(ii,jj) = SIZE;

      /***********************************************************************/
      /* Look at input file for specification of this entry.                 */
      /***********************************************************************/
      (void)fscanf(file, "%d", &xx);

      if ((xx < 0) ||
          (xx > SIZE))
      {
        printf("Bad egg %d in input file at %d %d\n", xx, ii+1, jj+1);
        rc = FALSE;
        goto EXIT;
      }

      if (xx > 0)
      {
        /*********************************************************************/
        /* We have a known entry. Update our array thusly.                   */
        /*********************************************************************/
        for (kk=0;kk<SIZE;kk++)
        {
          sudoku[ii][jj][kk] = FALSE;
        }
        sudoku[ii][jj][xx-1] = TRUE;

        /*********************************************************************/
        /* Set up the candidate count.                                       */
        /*********************************************************************/
        CAND_COUNT(ii,jj) = 1;

        /*********************************************************************/
        /* Set up the unique entry value.                                    */
        /*********************************************************************/
        FIXED_VAL(ii,jj) = xx;
      }
    }
  }

EXIT:

  return (rc);
}

static int attempt_to_solve(void)
{
  int num_known;
  int last_num_known = 0;
  int ii = 0;
  int jj = 0;
  int ll;
  int mm;
  int success = TRUE;
  int saved_entry[SIZE+2];
  int saved_sudoku[SIZE][SIZE][SIZE+2];
  int done;

  /***************************************************************************/
  /* Increment recursion depth count.  We dump this info on completion for   */
  /* curiosity's sake.                                                       */
  /***************************************************************************/
  cur_rec_depth++;

  if (max_rec_depth < cur_rec_depth)
  {
    /*************************************************************************/
    /* We need to update the deepest-ever recursion statistic.               */
    /*************************************************************************/
    max_rec_depth = cur_rec_depth;
  }

  while (((num_known = number_known()) > last_num_known) &&
         (num_known < (SIZE*SIZE)))
  {
    /*************************************************************************/
    /* Iterate applying the logical conditions of Sudoku until we have no    */
    /* more effect on the number of known entries.                           */
    /*************************************************************************/
    last_num_known = num_known;

    /*************************************************************************/
    /* Wind through each entry looking for restrictions applied by each of   */
    /* its row, column and block.                                            */
    /*************************************************************************/
    for (ii=0; ii < SIZE; ii++)
    {
      for (jj=0; jj < SIZE; jj++)
      {
        if (CAND_COUNT(ii,jj) > 1)
        {
          /*******************************************************************/
          /* Check for constrictions applied by the row, column and block.   */
          /*******************************************************************/
          proc_row(ii,jj);
          proc_col(ii,jj);
          proc_block(ii,jj);
        }
      }
    }
  }

  /*************************************************************************/
  /* Check that we have a consistent array still.  If not, we have failed  */
  /* and an outer recursion will backtrack and guess a different value for */
  /* an entry.                                                             */
  /*************************************************************************/
  if (do_check() == FALSE)
  {
    success = FALSE;
    goto EXIT;
  }

  if (num_known == (SIZE * SIZE))
  {
    /*************************************************************************/
    /* We've solved the puzzle so return with success.                       */
    /*************************************************************************/
    goto EXIT;
  }

  /***************************************************************************/
  /* Not got a complete solution yet so we have to start guessing.  Look for */
  /* the first entry in the array that has more than one possible value.     */
  /***************************************************************************/
  done = FALSE;
  for (ii=0;ii<SIZE;ii++)
  {
    for (jj=0;jj<SIZE;jj++)
    {
      if (CAND_COUNT(ii,jj) > 1)
      {
        done = TRUE;
        break;
      }
    }

    if (done == TRUE)
    {
      break;
    }
  }

  /***************************************************************************/
  /* Save off a copy the entry candidate list that we're going to use.       */
  /* We're going to overwrite the data soon but we need to use it.           */
  /***************************************************************************/
  memcpy(saved_entry, sudoku[ii][jj], sizeof(sudoku[ii][jj]));

  /***************************************************************************/
  /* Wind through each candidate attempting to solve the puzzle by trying to */
  /* solve it using each of them in turn.                                    */
  /***************************************************************************/
  for (ll=0;ll<SIZE;ll++)
  {
    if (saved_entry[ll] == TRUE)
    {
      /***********************************************************************/
      /* Save the current partially-completed sudoku.                        */
      /***********************************************************************/
      memcpy(saved_sudoku, sudoku, sizeof(sudoku));

      /***********************************************************************/
      /* Found a candidate.  Set up the entry to make it look as though      */
      /* we've decided the value of this entry.                              */
      /***********************************************************************/
      for (mm=0;mm<SIZE;mm++)
      {
        sudoku[ii][jj][mm] = FALSE;
      }
      sudoku[ii][jj][ll] = TRUE;

      /***********************************************************************/
      /* Set the candidate count to 1.                                       */
      /***********************************************************************/
      CAND_COUNT(ii,jj) = 1;

      /*********************************************************************/
      /* Set up the unique entry value.                                    */
      /*********************************************************************/
      FIXED_VAL(ii,jj) = ll+1;

      /***********************************************************************/
      /* Recursively attempt to solve the grid now we've fixed an extra      */
      /* point as an educated guess.                                         */
      /***********************************************************************/
      success = attempt_to_solve();

      if (success == TRUE)
      {
        /*********************************************************************/
        /* We have a solution so quit.                                       */
        /*********************************************************************/
        goto EXIT;
      }
      else
      {
        /*********************************************************************/
        /* Restore the old partially-completed sudoku from before we failed. */
        /*********************************************************************/
        memcpy(sudoku, saved_sudoku, sizeof(sudoku));
      }
    }
  }

EXIT:

  /***************************************************************************/
  /* Decrement the current number of recursions.                             */
  /***************************************************************************/
  cur_rec_depth--;

  return (success);
}

static void proc_row(int ii, int jj)
{
  int kk;

  /*********************************************************************/
  /* Check row containing this entry for restrictions we can apply to  */
  /* the current entry.                                                */
  /*********************************************************************/
  for (kk=0; kk < SIZE; kk++)
  {
    if ((jj != kk) &&
        (CAND_COUNT(ii,kk) == 1) &&
        ((CAND_COUNT(ii,jj)) > 1))
    {
      if ((sudoku[ii][jj][FIXED_VAL(ii,kk) - 1]) == TRUE)
      {
        /*********************************************************************/
        /* Eliminate this candidate.                                         */
        /*********************************************************************/
        sudoku[ii][jj][FIXED_VAL(ii,kk) - 1] = FALSE;

        /*********************************************************************/
        /* Decrement the candidate count.                                    */
        /*********************************************************************/
        CAND_COUNT(ii,jj)--;

        if (CAND_COUNT(ii,jj) == 1)
        {
          /*******************************************************************/
          /* We have uniqueness.  Update the unique value slot.              */
          /*******************************************************************/
          FIXED_VAL(ii,jj) = unique_value(ii,jj);
        }
      }
    }
  }
  return;
}

static void proc_col(int ii, int jj)
{
  int kk;

  /*********************************************************************/
  /* Check column containing this entry for restrictions we can apply  */
  /* to the current entry.                                             */
  /*********************************************************************/
  for (kk=0; kk < SIZE; kk++)
  {
    if ((ii != kk) &&
        (CAND_COUNT(kk,jj) == 1) &&
        (CAND_COUNT(ii,jj) > 1))
    {
      if (sudoku[ii][jj][FIXED_VAL(kk,jj) - 1] == TRUE)
      {
        /*********************************************************************/
        /* Eliminate this value.                                             */
        /*********************************************************************/
        sudoku[ii][jj][FIXED_VAL(kk,jj) - 1] = FALSE;

        /*********************************************************************/
        /* Decrement the candidate count.                                    */
        /*********************************************************************/
        CAND_COUNT(ii,jj)--;

        if (CAND_COUNT(ii,jj) == 1)
        {
          /*******************************************************************/
          /* We have uniqueness.  Update the unique value slot.              */
          /*******************************************************************/
          FIXED_VAL(ii,jj) = unique_value(ii,jj);
        }
      }
    }
  }
  return;
}

static void proc_block(int ii, int jj)
{
  int xx;
  int yy;
  int kk;
  int ll;

  /***************************************************************************/
  /* Find the top left-hand corner of the block.                             */
  /***************************************************************************/
  xx = (ii/3)*3;
  yy = (jj/3)*3;

  /***************************************************************************/
  /* Wind through each block entry.  If it is fixed then we know that the    */
  /* target entry can't be that value.                                       */
  /***************************************************************************/
  for (kk=xx; kk<(xx+BLOCK_SIZE); kk++)
  {
    for (ll=yy; ll<(yy+BLOCK_SIZE); ll++)
    {
      /***********************************************************************/
      /* We're on the next value in the block.                               */
      /***********************************************************************/
      if (((kk != ii) ||
           (ll != jj)) &&
           (CAND_COUNT(kk,ll) == 1))
      {
        if (sudoku[ii][jj][FIXED_VAL(kk,ll) - 1] == TRUE)
        {
          /*******************************************************************/
          /* It's a fixed value.  Update the sudoku array thusly.            */
          /*******************************************************************/
          sudoku[ii][jj][FIXED_VAL(kk,ll) - 1] = FALSE;

          /*******************************************************************/
          /* Decrement the candidate count.                                  */
          /*******************************************************************/
          CAND_COUNT(ii,jj)--;

          if (CAND_COUNT(ii,jj) == 1)
          {
            /*****************************************************************/
            /* We have uniqueness.  Update the unique value slot.            */
            /*****************************************************************/
            FIXED_VAL(ii,jj) = unique_value(ii,jj);
          }
        }
      }
    }
  }
  return;
}

static int number_known(void)
{
  int ii;
  int jj;
  int nn = 0;

  for (ii=0;ii<SIZE;ii++)
  {
    for (jj=0;jj<SIZE;jj++)
    {
      if (CAND_COUNT(ii,jj) == 1)
      {
        /*********************************************************************/
        /* This entry is fixed so increment our count.                       */
        /*********************************************************************/
        nn++;
      }
    }
  }

  return nn;
}

static int do_check(void)
{
  int ii;
  int jj;
  int kk;
  int xx;
  int yy;
  int ll;
  int success = TRUE;

  for (ii=0; ii < SIZE; ii++)
  {
    for (jj=0; jj < SIZE; jj++)
    {
      /***********************************************************************/
      /* Check whether the entry has run out of candidates.                  */
      /***********************************************************************/
      if (CAND_COUNT(ii,jj) == 0)
      {
        success = FALSE;
        goto EXIT;
      }

      /***********************************************************************/
      /* Check horizontals for duplicates.  The ordering of the tests is     */
      /* optimized to do the most unlikely ones first.                       */
      /***********************************************************************/
      for (kk=0; kk < SIZE; kk++)
      {
        if ((CAND_COUNT(ii,kk) == 1) &&
            (CAND_COUNT(ii,jj) == 1) &&
            (jj != kk) &&
            (FIXED_VAL(ii,kk) == FIXED_VAL(ii,jj)))
        {
          success = FALSE;
          goto EXIT;
        }
      }

      /***********************************************************************/
      /* Check verticals for duplicates.  The ordering of the tests is       */
      /* optimized to do the most unlikely ones first.                       */
      /***********************************************************************/
      for (kk=0; kk < SIZE; kk++)
      {
        if ((CAND_COUNT(kk,jj) == 1) &&
            (CAND_COUNT(ii,jj) == 1) &&
            (ii != kk) &&
            (FIXED_VAL(kk,jj) == FIXED_VAL(ii,jj)))
        {
          success = FALSE;
          goto EXIT;
        }
      }

      /***********************************************************************/
      /* Check block containing this entry.  Find the top left-hand corner   */
      /* of the block.                                                       */
      /***********************************************************************/
      xx = (ii/3)*3;
      yy = (jj/3)*3;

      /***********************************************************************/
      /* Wind through all entries in the block.                              */
      /***********************************************************************/
      for (kk=xx; kk<(xx+BLOCK_SIZE); kk++)
      {
        for (ll=yy; ll<(yy+BLOCK_SIZE); ll++)
        {
          /*******************************************************************/
          /* We're on the next value in the block.  The order of the tests   */
          /* here is optimized to check the most unlikely conditions first.  */
          /*******************************************************************/
          if ((CAND_COUNT(kk,ll) == 1) &&
              (CAND_COUNT(ii,jj) == 1) &&
              ((kk != ii) || (ll != jj)) &&
              (FIXED_VAL(kk,ll) == FIXED_VAL(ii,jj)))
          {
            /*****************************************************************/
            /* Two different entries have been uniquely identified but are   */
            /* the same.  This is a bogus block.  Bail out.                  */
            /*****************************************************************/
            success = FALSE;
            goto EXIT;
          }
        }
      }
    }
  }

EXIT:

  return success;
}

/*****************************************************************************/
/* Prints out the sudoku solution array.                                     */
/*****************************************************************************/
static void dump(void)
{
  int ii;
  int jj;

  printf("\n");

  printf("   1 2 3  4 5 6  7 8 9\n");
  printf("   -----  -----  -----\n");

  for (ii=0;ii<SIZE;ii++)
  {
    printf("%d| ", ii+1);
    for (jj=0;jj<SIZE;jj++)
    {
      if (CAND_COUNT(ii,jj) > 1)
      {
        printf("0 ");
      }
      else
      {
        printf("%d ", FIXED_VAL(ii,jj));
      }

      if ((jj % BLOCK_SIZE) == (BLOCK_SIZE - 1))
      {
        printf(" ");
      }
    }
    printf("\n");
    if ((ii % BLOCK_SIZE) == (BLOCK_SIZE - 1))
    {
      printf("\n");
    }
  }

 #ifdef DEBUG
  for (ii=0;ii<SIZE;ii++)
  {
    for (jj=0;jj<SIZE;jj++)
    {
      printf("%d ", CAND_COUNT(ii,jj));

      if ((jj % BLOCK_SIZE) == (BLOCK_SIZE - 1))
      {
        printf(" ");
      }
    }
    printf("\n");
    if ((ii % BLOCK_SIZE) == (BLOCK_SIZE - 1))
    {
      printf("\n");
    }
  }
 #endif
}

/*****************************************************************************/
/* This function returns the unique value that an entry with only one        */
/* candidate has.                                                            */
/*****************************************************************************/
static int unique_value(int xx, int yy)
{
  int ii;
  int output = SIZE+1;

  ASSERT(CAND_COUNT(xx,yy) == 1);

  for (ii=0; ii<SIZE; ii++)
  {
    if (sudoku[xx][yy][ii] == TRUE)
    {
      output = ii+1;
      goto EXIT;
    }
  }

EXIT:

  ASSERT(output < (SIZE+1));

  return (output);
}
