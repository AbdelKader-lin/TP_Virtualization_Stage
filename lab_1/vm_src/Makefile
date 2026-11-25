GCC=gcc -g

all: main

run: 
	./bin/main

main:
	@mkdir -p bin
	$(GCC) ../vm_manager/manager.c ../load_manager/loader.c src/main.c -o bin/main 

clean:
	rm bin/*


