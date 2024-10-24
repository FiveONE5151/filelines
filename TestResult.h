#pragma once
#include <cstdint>
struct TestResult
{
    uint32_t total_line_num, most_freq_len, most_freq_len_linenum;
    int64_t time;
    TestResult(uint32_t total_line_num, uint32_t most_freq_len, uint32_t most_freq_len_linenum, int64_t time) : total_line_num(total_line_num), most_freq_len(most_freq_len), most_freq_len_linenum(most_freq_len_linenum), time(time) {};
};