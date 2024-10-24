#include "TestResult.h"
#include "find_most_freq.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <chrono>
using namespace std;
using namespace std::chrono;

const char testFile[14] = "test/test.txt";
#define BLOCK_SIZE 64
void filelines_blocked(const char *filepath, uint32_t *total_line_num, uint32_t *line_num, const int blockSize = 64)
{
    int handle;
    if ((handle = open(filepath, O_RDONLY)) < 0)
        return;
    char *bp = (char *)malloc(blockSize);
    if (bp == NULL)
    {
        close(handle);
        return;
    }
    int cur_len = 0;
    while (1)
    {
        int bytes_read = read(handle, bp, blockSize);
        if (bytes_read <= 0)
            break;
        for (int i = 0; i < bytes_read; i++)
        {
            if (bp[i] == '\n')
            {                        // 读到换行符, 代表一行结束
                (*total_line_num)++; // 行数增加
                if (cur_len >= MAX_LEN)
                    line_num[MAX_LEN - 1]++; // 如果这一行长度超过1024个字符
                else
                    line_num[cur_len]++; // 对应长度的行的个数增加
                cur_len = 0;
            }
            else
            {
                cur_len++; // 计算一行的字符数量
            }
        }
    }
    close(handle);
}
TestResult testBlocked(const int blockSize = 64)
{
    uint32_t most_freq_len, most_freq_len_linenum;
    uint32_t line_num[MAX_LEN] = {0};
    uint32_t total_line_num;
    total_line_num = 0;
    auto start = high_resolution_clock::now();
    filelines_blocked(testFile, &total_line_num, line_num, blockSize);
    find_most_freq_line(line_num, &most_freq_len, &most_freq_len_linenum);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    return TestResult(total_line_num, most_freq_len, most_freq_len_linenum, duration.count());
}
int main(int argc, char *argv[])
{
    int blockSize[6] = {64, 256, 1024, 4 * 1024, 16 * 1024, 256 * 1024};
    for (size_t i = 0; i < 6; i++)
    {
        TestResult testResult = testBlocked(blockSize[i]);
        cout << setw(15) << "Block Size" << setw(15) << "Time" << endl;
        cout << setw(15) << blockSize[i] << setw(15) << testResult.time << endl;
        printf("%d %d %d\n", testResult.total_line_num, testResult.most_freq_len, testResult.most_freq_len_linenum);
    }
}
