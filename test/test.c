#include <stdio.h>

double* result(double*);

//double result(int i);
//int result(int i);

int main()
{
    const unsigned int count = 25;

    double in[count];
    int i;

    for (i = 0; i < count; ++i)
        in[i] = i;

    double *out = result(in);

    printf("out: ");
    for (i = 0; i < count; ++i)
        printf("%f\n", out[i]);

    return 0;
}
