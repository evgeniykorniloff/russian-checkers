#
#
#      Make for russian checkers project
#
#
rcheckers: main.o bitbrd.o checkers.o genmov.o hash.o score.o search.o utils.o timecont.o
	gcc -o rcheckers main.o bitbrd.o checkers.o genmov.o hash.o score.o search.o utils.o timecont.o
main.o: main.c
	gcc -c main.c
bitbrd.o: bitbrd.c
	gcc -c bitbrd.c
checkers.o: checkers.c
	gcc -c checkers.c
genmov.o: genmov.c
	gcc -c genmov.c -O3
hash.o: hash.c
	gcc -c hash.c
score.o: score.c
	gcc -c score.c -O3
search.o: search.c
	gcc -c search.c -O3
utils.o: utils.c
	gcc -c  utils.c
timecont.o: timecont.c
	gcc -c timecont.c
clean:
	rm *.o 

