name = untitled_shell
version = 0.0.1
cc = gcc

common_define = -DVERSION=\"$(version)\" -DPROGRAM_NAME=\"$(name)\" -Wall

all:
	$(cc) -o $(name) *.c $(common_define)
dev:
	$(cc) -g -o $(name)_dev *.c $(common_define) -DLOGGING_LEVEL=LOG_DEBUG -DPRINT_AST
clean:
	rm -f $(name) $(name)_dev