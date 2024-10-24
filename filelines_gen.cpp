#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#include "find_most_freq.h"

uint32_t line_num[MAX_LEN];
uint32_t total_line_num;
void init_line_num(uint32_t *total_line_num, uint32_t *line_num)
{
    for (int i = 0; i < MAX_LEN; i++)
        line_num[i] = 0;
    line_num[0] = 5558888;
    line_num[1] = 8888888;
    line_num[2] = 14448888;
    line_num[7] = 99988888;
    line_num[12] = 8888888;
    line_num[13] = 29888888;
    line_num[14] = 98888888;
    for (int i = 0; i < MAX_LEN; i++)
        *total_line_num += line_num[i];
}
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s filepath", argv[0]);
        return -1;
    }

    int handle;
    if ((handle = open(argv[1], O_CREAT | O_WRONLY | O_TRUNC,
                       S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) < 0)
        return -1;

    init_line_num(&total_line_num, line_num);
    char buf[26];
    for (int i = 0; i < 26; i++)
        buf[i] = i + 'a';
    char wbuf[MAX_LEN];
    while (1)
    {
        int all_zero = 1;
        for (int i = 0; i < MAX_LEN; i++)
        {
            if (line_num[i] != 0)
            {
                all_zero = 0;
                memcpy(wbuf, buf, i);
                wbuf[i] = 0x0a;
                int write_num;
                write_num = write(handle, wbuf, i + 1);
                line_num[i]--;
            }
        }
        if (all_zero == 1)
            break;
    }
    printf("%d\n", total_line_num);
    close(handle);
}