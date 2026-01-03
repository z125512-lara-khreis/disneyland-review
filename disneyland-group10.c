#include <stdio.h>

int view_data(const char *file)
{
    FILE *fp;
    char ch;

    fp = fopen("disneylandreview.csv", "r");
    if (fp == NULL)
    {
        printf("Can't open file!\n");
    }
    else
    {
        while (ch != EOF)
        {
           ch = getc(fp);
           putchar(ch);
        }
    }
    fclose(fp);
    return 0;
}

int main(void)
{
    view_data("disneyland.csv");

    return 0;
}
