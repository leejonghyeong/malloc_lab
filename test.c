#include<stdio.h>

int **array[10] = {NULL};

int main(){
    int x = 1;
    int *y = &x;
    
    array[1] = &y;
    printf("%d\n",**array[1]);


    printf("hellos");

    printf("%d\n", *y);
    printf("%d\n", 2056|0x1);
    printf("%ld\n", 2057 & ~0xfL);
    printf("%d\n", 2056 & ~0xf);
     printf("%d\n", 2057 & ~0x7);

    return 0;
}