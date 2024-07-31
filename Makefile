all: fscli

fscli:
	gcc -o fscli main.c fs.c

clean:
	rm -f fscli *.o

run: fscli
	./fscli
