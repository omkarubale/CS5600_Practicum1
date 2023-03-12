all: main

main: pm_heap.c pm_heap.h test.c
	gcc -w ./test.c ./pm_heap.c -o "pm_heap_run"

clean:
	rm -f pm_heap_run
