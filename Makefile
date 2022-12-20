#Memory in KB
MEMORY = 102400000
N = 100000000000
RAMA = out/ramanujan
CC = clang
CFLAGS = -O3 -Wall
# PERFFLAGS = -e cycles -e instructions -e branch-misses -e LLC-load-misses -e LLC-store-misses
PERFFLAGS = -e cpu_core/cycles/ -e cpu_core/instructions/ -e cpu_core/branch-misses/ -e cpu_core/LLC-load-misses/ -e cpu_core/LLC-store-misses/

SOURCES = ramasort.c ramanujan.c Makefile

.PHONY: bench dist

bench: $(RAMA)
	taskset 255 perf stat $(PERFFLAGS) ./$(RAMA) $(N)

out:
	mkdir out/

clean:
	rm -rd out

out/ramanujan: ramanujan.c out
	$(CC) $(CFLAGS) $< -lm -o $@

out/ramasort: ramasort.c out
	$(CC) $(CFLAGS) $< -lm -o $@

out/ramanujan%: ramanujan%.c out
	$(CC) $(CFLAGS) $< -lm -o $@

dist: ../ramanujan.tar.gz

../ramanujan.tar.gz: $(SOURCES)
	cd .. && tar cfz ramanujan.tar.gz ramanujan/ramasort.c ramanujan/ramanujan.c ramanujan/Makefile
