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
    const int BYTE_SIZE = 16; // 一个向量有16个字节
    if ((handle = open(filepath, O_RDONLY)) < 0)
        return;
    char *bp = nullptr;
    int err = posix_memalign((void **)&bp, BYTE_SIZE * sizeof(char), blockSize);
    if (err)
    {
        close(handle);
        return;
    }
    int cur_len = 0;

    const __m128i endline = _mm_set1_epi8('\n');
    while (1)
    {
        int bytes_read = read(handle, bp, blockSize);
        if (bytes_read <= 0)
            break;
        for (int i = 0; i < bytes_read; i += BYTE_SIZE)
        {
            const __m128i s = _mm_load_si128((const __m128i *)(bp));
            char *temp = bp;
            __m128i x = _mm_cmpeq_epi8(s, endline);
            unsigned short r = (unsigned short)(_mm_movemask_epi8(x));
            if (r == 0)
            {
                cur_len += BYTE_SIZE;
                continue;
            }
            unsigned long vec_offset = 0; // 记录向量内的偏移量
            while (true)
            {
                int ffs = __builtin_ffs(r); // 记录掩码中第一个1的偏移量, 也是新一行的字符数(如果这一行全部都在向量内)
                if (ffs == 0)
                {
                    cur_len += BYTE_SIZE - vec_offset - 1; // 加上向量内剩余的字符数
                    break;
                }
                vec_offset += ffs - 1;
                cur_len += ffs;
                (*total_line_num)++; // 行数增加
                if (cur_len >= MAX_LEN)
                    line_num[MAX_LEN - 1]++; // 如果这一行长度超过1024个字符
                else
                    line_num[cur_len]++; // 对应长度的行的个数增加
                r >>= ffs;
                cur_len = 0;
            }
        }
    }
    close(handle);
}
const char testFile[19] = "test/testDebug.txt";
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