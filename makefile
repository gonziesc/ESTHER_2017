OBJ := cpu.o kernel.o filesystem.o memoria.o consola.o
# CURRENT_DIR := $(shell pwd)
DIR := $(CURDIR)
HEADERS: = -I"$(DIR)/commons/commons"  -I"$(DIR)/commons/commons/collections"  -I"$(DIR)/Kernel" -I"$(DIR)/Memoria"  -I"$(DIR)/Consola" -I"$(DIR)/Cpu" -I"$(DIR)/FIleSystem"  -I"$(DIR)/ansisop/parser/parser"  -I "$(DIR)/shared"
LIBPATH := -L "$(DIR)/ansisop/commons/src/build" -L "$(DIR)/ansisop/parser/build" -L "$(DIR)/ansisop/parser/parser"  -L "$(DIR)/shared"
LIBS := -lcommons -lpthread -lparser-ansisop -lm -lshared

CC:= gcc -w -g
CFLAGS:= -std=c11 &(HEADERS)

# ALL

all: clean kernel cpu memoria consola filesystem

kernel: kernel.o $(CC) $$(LIBPATH) kernel.o -o kernel$(LIBS)

kernel.o: $(CC) $(CFLAGS) -c $(DIR)/Kernel main.c -o main.o


cpu: cpu.o $$(CC) $(LIBPATH) cpu.o -o cpu$(LIBS)

cpu.o: $(CC) $(CFLAGS) -c $(DIR)/Cpu/main.c -o main.o

consola: consola.o $$(CC) $(LIBPATH) consola.o -o consola$(LIBS)

consola.o: $(CC) $(CFLAGS) -c $(DIR)/Consola/main.c -o main.o

filesystem: filesystem.o $$(CC) $$(LIBPATH) filesystem.o -o filesystem$(LIBS)

filesystem.o: $(CC) $(CFLAGS) -c $(DIR)/FIleSystem/main.c -o main.o

memoria: memoria.o $$(CC) $$(LIBPATH) memoria.o -o memoria$(LIBS)

memoria.o: $(CC) $(CFLAGS) -c $(DIR)/Memoria/main.c -o main.o

libclean:
	$(MAKE) uninstall -C $(DIR)/ansisop/parser


