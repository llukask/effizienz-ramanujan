/* Ramanujan numbers are numbers that can be formed by adding two
   cubes of integer numbers in two (or more) different ways, i.e.
   i^3+j^3 = k^3+l^3
   See <http://www.durangobill.com/Ramanujan.html>

   This program counts the Ramanujan numbers up to a given
*/

#include <locale.h>
#include <malloc.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRIDE (long)(1.5E9)
#define TABLE_SIZE (long)(1048576)

#ifdef DEBUG
#define DEBUG_PRINT(x) printf x
#else
#define DEBUG_PRINT(x) \
  do                   \
  {                    \
  } while (0)
#endif

long cube(long n) { return n * n * n; }
long min(long a, long b) { return a <= b ? a : b; }
long max(long a, long b) { return a >= b ? a : b; }

int comp_sums(const void *a, const void *b)
{
  return (*(long *)a - *(long *)b);
}

int main(int argc, char **argv)
{
  long n;
  char *endptr;
  long from, i, j;
  long table_idx;
  long table_size = TABLE_SIZE;
  long stride = (long)STRIDE;
  long count = 0;
  long checksum = 0;
  long total_iterations = 0;
  long segments = 0;

  long max_table_idx = 0;

  if (argc != 2)
    goto usage;
  n = strtol(argv[1], &endptr, 10);
  if (*endptr != '\0')
    goto usage;

  setlocale(LC_NUMERIC, "");

  printf("finding all ramanujan numbers up to %'ld\n"
         "using the following configuration:\n"
         "  table size: %'9ld\n"
         "      stride: %'9ld\n",
         n, table_size, stride);

  long *table = calloc(table_size, sizeof(long));

  for (from = 0; from <= n; from += stride)   // from exclusive, to inclusive
  {
    long to = min(n, from + stride);
    i = (long)floor(cbrt(to-1));
    long i_min = (long)ceil(cbrt((from +1) * 0.5));

    //printf("calculating from ]%'ld, %'ld] with i range of [%'ld, %'ld]\n", from, to, i_min, i);

#ifdef DEBUG
    long start_iterations = total_iterations;
#endif

    table_idx = 0;
    long j_min = 1;
    for (; i >= i_min; i--)
    {
      char found_valid = 0;
      j = j_min;
      long i_cube = cube(i);
      long sum = i_cube + cube(j);
      while (sum <= to && j < i)
      {
        total_iterations++;
        if (from < sum && sum <= to)
        {
          if (!found_valid)
          {
            found_valid = 1;
            j_min = j;
          }

          table[table_idx++] = sum;
        }
        j++;
        sum = i_cube + cube(j);
      }
    }

    qsort(table, table_idx, sizeof(long), comp_sums);

    for (int k = 1; k < table_idx; k++)
    {
      if (table[k - 1] == table[k])
      {
        count++;
        checksum += table[k];
        while (k < table_idx - 1 && table[k] == table[k + 1])
          k++;
      }
    }

    if (table_idx > max_table_idx)
    {
      max_table_idx = table_idx;
    }

    segments++;

    DEBUG_PRINT(("segment %'4ld: iterations = %'6ld\n", segments,
                 total_iterations - start_iterations));
  }

  printf("  %'ld Ramanujan numbers up to %'ld, check sum=%'ld\n\nmax used table slots=%'ld ",
         count, n, checksum, max_table_idx);

  return 0;

usage:
  fprintf(stderr, "usage: %s <n>\n", argv[0]);
  exit(1);
}
