#include <stdio.h>

double* result(double*);

//double result(int i);
//int result(int i);

int main()
{
    double in[10];
    int i;

    for (i = 0; i < 10; ++i)
        in[i] = i;

    double *out = result(in);

    printf("out: ");
    for (i = 0; i < 10; ++i)
        printf("%f,  ", out[i]);
    printf("\n");

    return 0;
}
