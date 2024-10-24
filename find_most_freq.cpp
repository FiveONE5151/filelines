#include "find_most_freq.h"
void find_most_freq_line(uint32_t *line_num, uint32_t *most_freq_len, uint32_t *most_freq_len_linenum)
{
    int t_linenum = 0;
    int t_len = 0;
    // 遍历linenum数组, 去找到出现次数最多的长度的行的数量
    for (int i = 0; i < MAX_LEN; i++)
    {
        if (line_num[i] > t_linenum)
        {
            t_linenum = line_num[i];
            t_len = i;
        }
    }
    *most_freq_len = t_len;
    *most_freq_len_linenum = t_linenum;
}