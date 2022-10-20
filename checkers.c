#include <stdio.h>
#include <stdlib.h>

extern  int loop(int argc, char *argv[]);
int main(int argc, char **argv)
{
        setbuf(stdout, NULL);
        setbuf(stdin, NULL);
        loop(argc,argv);
        return 0;
}
