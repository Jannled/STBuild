#include "test.h"

#include <stdio.h>

int main(int argc, char** argv)
{
    printf("Hello World\r\n");
    printf("Kreisfläche mit Radius %.2f = %.2f\r\n", 6.0f, surface(6.0f));
}