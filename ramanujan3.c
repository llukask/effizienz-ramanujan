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

struct entry {
  long value;
  long count;
};

struct vec {
  size_t size;
  size_t capacity;
  struct entry *entries;
};

struct set {
  size_t size;
  size_t mask;
  size_t pages_used;
  size_t total_capacity;
  struct vec *pages;
};

struct vec *lookup_page(struct set *s, long value) {
  size_t page_idx = (value * 31) & s->mask;
  return &s->pages[page_idx];
}

long add(struct set *s, long value) {
  struct vec *page = lookup_page(s, value);

  int i;
  for (i = 0; i < page->size; i++) {
    if (page->entries[i].value == value) {
      return ++page->entries[i].count;
    }
  }

  if (i >= page->capacity) {
    size_t new_cap = 2 * page->capacity;
    if (page->capacity == 0) {
      s->pages_used++;
      new_cap = 1;
    }

    s->total_capacity += (new_cap - page->capacity);

    page->entries = realloc(page->entries, new_cap * sizeof(struct entry));

    page->capacity = new_cap;
  }

  page->size++;
  page->entries[i].count = 1;
  page->entries[i].value = value;
  return 1;
}

struct set init_set(size_t pages) {
  struct set s;
  s.size = pages;
  s.mask = pages - 1;
  s.pages_used = 0;
  s.total_capacity = 0;

  s.pages = calloc(pages, sizeof(struct vec));

  return s;
}

size_t set_size(struct set *s) {
  size_t size = sizeof(struct set);
  size += s->size * sizeof(struct vec);
  size += s->total_capacity * sizeof(struct entry);

  return size;
}

long cube(long n) { return n * n * n; }

size_t size_table(long n)
/* compute the table size so it is not too densely or too sparsely occupied
   and is a power of 2 */
{
  return 1 << (long)(log((double)n) * (2.0 / (3.0 * log(2.0))));
}

long min(long a, long b) { return a <= b ? a : b; }
long max(long a, long b) { return a >= b ? a : b; }

int main(int argc, char **argv) {
  long n;
  char *endptr;
  long i, j;
  long count = 0;
  size_t table_size;
  long checksum = 0;
  long total_iterations = 0;
  long segments = 0;

  if (argc != 2)
    goto usage;
  n = strtol(argv[1], &endptr, 10);
  if (*endptr != '\0')
    goto usage;

  setlocale(LC_NUMERIC, "");

  long stride = (long)5.0E9;

  table_size = 262144; // size_table(stride) >> 4;
  struct set s = init_set(table_size);

  for (i = 0; i <= n; i += stride) {
    long to = min(n, i + stride);
    long k = (long)ceil(cbrt(to));
    long kmin = (long)(floor(cbrt(i)) * 0.5);

    long min_j = 1;
    for (; k >= kmin; k--) {
      char found_valid = 0;
      for (j = min_j; cube(k) + cube(j) <= to && j < k; j++) {
        total_iterations++;
        long sum = cube(k) + cube(j);
        if (i <= sum && sum < to) {
          if (!found_valid) {
            found_valid = 1;
            min_j = j;
          }
          if (add(&s, sum) == 2) {
            count++;
            checksum += sum;
          }
        }
      }
    }

    for (int p = 0; p < s.size; p++) {
      s.pages[p].size = 0;
    }

    segments++;
  }

  printf("%'ld Ramanujan numbers up to %'ld, check sum=%'ld\n\n"
         "occupation = %'15ld\n"
         "      size = %'15ld\n"
         "      sums = %'15ld\n"
         "  segments = %'15ld\n"
         "  set_size = %'15ld B\n", // table size in bytes
         count, n, checksum, s.pages_used, table_size, total_iterations,
         segments, set_size(&s));

  return 0;

usage:
  fprintf(stderr, "usage: %s <n>\n", argv[0]);
  exit(1);
}
