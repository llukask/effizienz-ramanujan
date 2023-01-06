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

#define STRIDE (long)(0.4E9)
#define TABLE_SIZE (long)(16384)

#ifdef DEBUG
#define DEBUG_PRINT(x) printf x
#else
#define DEBUG_PRINT(x) \
  do                   \
  {                    \
  } while (0)
#endif

struct entry
{
  long value;
  long count;
};

struct bucket
{
  size_t size;
  size_t capacity;
  struct entry *entries;
};

struct set
{
  size_t size;
  size_t mask;
  size_t buckets_used;
  size_t total_capacity;
  struct bucket *buckets;
};

struct bucket *lookup_bucket(struct set *s, long value)
{
  size_t bucket_idx = value & s->mask;
  return &s->buckets[bucket_idx];
}

long add(struct set *s, long value)
{
  struct bucket *bucket = lookup_bucket(s, value);

  int i;
  for (i = 0; i < bucket->size; i++)
  {
    if (bucket->entries[i].value == value)
    {
      return ++bucket->entries[i].count;
    }
  }

  if (i >= bucket->capacity)
  {
    size_t new_cap = 2 * bucket->capacity;
    if (bucket->capacity == 0)
    {
      s->buckets_used++;
      new_cap = 1;
    }

    s->total_capacity += (new_cap - bucket->capacity);

    bucket->entries = realloc(bucket->entries, new_cap * sizeof(struct entry));
    if (bucket->entries == NULL)
    {
      printf("realloc failed!\n");
    }

    bucket->capacity = new_cap;
  }

  bucket->size++;
  bucket->entries[i].count = 1;
  bucket->entries[i].value = value;
  return 1;
}

struct set init_set(size_t buckets)
{
  struct set s;
  s.size = buckets;
  s.mask = buckets - 1;
  s.buckets_used = 0;
  s.total_capacity = 0;

  s.buckets = calloc(buckets, sizeof(struct bucket));

  return s;
}

size_t set_size(struct set *s)
{
  size_t size = 0;
  size += s->size * sizeof(struct bucket);
  size += s->total_capacity * sizeof(struct entry);

  return size;
}

long cube(long n) { return n * n * n; }
long min(long a, long b) { return a <= b ? a : b; }
long max(long a, long b) { return a >= b ? a : b; }

int main(int argc, char **argv)
{
  long n;
  char *endptr;
  long from, i, j, j_min, i_cube, sum;
  long table_size = TABLE_SIZE;
  long stride = (long)STRIDE;
  long count = 0;
  long checksum = 0;
  long total_iterations = 0;
  long segments = 0;
  char found_valid;

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

  struct set s = init_set(table_size);

  for (from = 0; from <= n; from += stride) // from exclusive, to inclusive
  {
    long to = min(n, from + stride);
    i = (long)floor(cbrt(to - 1));
    long i_min = (long)ceil(cbrt((from + 1) * 0.5));

    // printf("calculating from ]%'ld, %'ld] with i range of [%'ld, %'ld]\n", from, to, i_min, i);

#ifdef DEBUG
    long start_iterations = total_iterations;
#endif

    j_min = 1;
    for (; i >= i_min; i--)
    {
      found_valid = 0;
      j = j_min;
      i_cube = cube(i);
      sum = i_cube + cube(j);
      while (sum <= to && j <= i)
      {
        total_iterations++;
        if (sum <= to && from < sum)
        {
          if (!found_valid)
          {
            found_valid = 1;
            j_min = j;
          }

          if (add(&s, sum) == 2)
          {
            count++;
            checksum += sum;
          }
        }
        j++;
        sum = i_cube + cube(j);
      }
    }

    for (int p = 0; p < s.size; p++)
    {
      s.buckets[p].size = 0;
    }

    segments++;

    DEBUG_PRINT(("segment %'4ld: iterations = %'6ld\n", segments,
                 total_iterations - start_iterations));
  }

  printf("  %'ld Ramanujan numbers up to %'ld, check sum=%'ld\n\n"
         "  occupation = %'15ld\n"
         "        size = %'15ld\n"
         "        sums = %'15ld\n"
         "    segments = %'15ld\n"
         "      stride = %'15ld\n"
         "    set_size = %'15ld B\n", // table size in bytes
         count, n, checksum, s.buckets_used, table_size, total_iterations,
         segments, stride, set_size(&s));

  for (int bucket = 0; bucket < s.size; bucket++)
  {
    free(s.buckets[bucket].entries);
  }

  free(s.buckets);

  return 0;

usage:
  fprintf(stderr, "usage: %s <n>\n", argv[0]);
  exit(1);
}
