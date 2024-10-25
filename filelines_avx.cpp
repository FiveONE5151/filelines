#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "find_most_freq.h"
#include <emmintrin.h>
#include <chrono>
#include <iostream>
using namespace std::chrono;
using namespace std;
void filelines_avx(const char *filepath, uint32_t *total_line_num, uint32_t *line_num, const int blockSize = 64)
{
    int handle;
    const int CHAR_SIZE = 16; // 一个向量有16个字节
    if ((handle = open(filepath, O_RDONLY)) < 0)
        return;
    char *bp = nullptr;
    int err = posix_memalign((void **)&bp, CHAR_SIZE * sizeof(char), blockSize);
    if (err)
    {
        close(handle);
        return;
    }
    int cur_len = 0;
    char *temp = bp;                             // 向量指针，指向检查的向量的起始位置
    const __m128i endline = _mm_set1_epi8('\n'); // 换行符向量，用于比较
    while (1)
    {
        int bytes_read = read(handle, bp, blockSize);

        if (bytes_read <= 0)
            break;

        // 遍历块内的char向量
        for (temp = bp; temp < bp + bytes_read; temp += CHAR_SIZE)
        {
            const __m128i s = _mm_loadu_si128((const __m128i *)(temp));

            __m128i x = _mm_cmpeq_epi8(s, endline);                    // 检查块内向量是否含有换行符，如果有那么对应的字节被设置为11111111
            unsigned short r = (unsigned short)(_mm_movemask_epi8(x)); // 提取x中每一个字节的最高位，也就是向量内为换行符的字节的bitmap

            // 向量内没有换行符
            if (r == 0)
            {
                cur_len += CHAR_SIZE; // 记录上这一行的字符数，累加到下一向量的统计
                continue;
            }
            unsigned long vec_offset = 0; // 记录换行符在向量内的偏移量

            // 检查向量内的换行符
            while (true)
            {
                int ffs = __builtin_ffs(r); // 查找向量内第一个换行符在bitmap中的索引，从1开始

                // 已经统计完向量内所有的换行符
                if (ffs == 0)
                {
                    cur_len += CHAR_SIZE - vec_offset; // 加上向量内剩余的字符数
                    break;                             // 退出，检查下一个向量
                }
                vec_offset += ffs;   // 下一行字符在向量内的起始位置
                cur_len += ffs - 1;  // 记录这一行的字符数
                (*total_line_num)++; // 行数增加
                if (cur_len >= MAX_LEN)
                    line_num[MAX_LEN - 1]++; // 如果这一行长度超过1024个字符
                else
                    line_num[cur_len]++; // 对应长度的行的个数增加
                r >>= ffs;               // 消除bitmap中已经检索到的换行符
                cur_len = 0;             // 重置行字符数，统计下一行
            }
        }
    }
    close(handle);
}
const char testFile[19] = "test/test.txt";
uint32_t line_num[MAX_LEN];
uint32_t total_line_num;

int main(int argc, char const *argv[])
{
    const int blockSize = 256 * 1024;
    uint32_t most_freq_len, most_freq_len_linenum;
    total_line_num = 0;
    auto start = high_resolution_clock::now();
    filelines_avx(testFile, &total_line_num, line_num, blockSize);
    find_most_freq_line(line_num, &most_freq_len, &most_freq_len_linenum);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    printf("%d %d %d\n", total_line_num, most_freq_len, most_freq_len_linenum);
    cout << "Time: " << duration.count() << "ms" << endl;
}