#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>
#include <unistd.h>
#include "find_most_freq.h"
#include <emmintrin.h>
#include <chrono>
#include <iostream>
#include <pthread.h>
using namespace std::chrono;
using namespace std;
uint32_t line_num[MAX_LEN];
const int BUFFER_SIZE = 50; // 缓冲区大小
#define mov(pos) (pos + 1) % BUFFER_SIZE
struct Product
{
    char *bp;
    int bytes_read;
    Product(char *bp = nullptr, int bytes_read = 0) : bp(bp), bytes_read(bytes_read) {};
};
struct ProductSoA
{
    char *bp[BUFFER_SIZE];
    int bytes_read[BUFFER_SIZE];
    ProductSoA()
    {
        for (size_t i = 0; i < BUFFER_SIZE; i++)
        {
            bp[i] = nullptr;
            bytes_read[i] = 0;
        }
    }
};
struct prodcons
{
    Product buffer[BUFFER_SIZE];
    pthread_mutex_t lock;
    pthread_cond_t notempty;
    pthread_cond_t notfull;
    int writepos, readpos;
};
struct prodcons buffer;

void init(struct prodcons *p)
{
    pthread_mutex_init(&(p->lock), NULL);
    pthread_cond_init(&(p->notempty), NULL);
    pthread_cond_init(&(p->notfull), NULL);
    p->readpos = 0;
    p->writepos = 0;
}
void put(struct prodcons *b, Product p)
{
    pthread_mutex_lock(&(b->lock));
    while (mov(b->writepos) == b->readpos) // 环形缓冲区, 缓冲区满的判断
    {
        pthread_cond_wait(&(b->notfull), &(b->lock)); // 缓冲区满, 等待消费者读取数据并唤醒
    }
    b->buffer[b->writepos] = p;
    b->writepos = mov(b->writepos);
    pthread_cond_signal(&(b->notempty));
    pthread_mutex_unlock(&(b->lock));
}

Product get(struct prodcons *b)
{
    pthread_mutex_lock(&(b->lock));
    while (b->writepos == b->readpos) // 环形缓冲区, 缓冲区空的判断
    {
        pthread_cond_wait(&(b->notempty), &(b->lock)); // 缓冲区空, 等待生产者写入数据并唤醒
    }
    Product p = b->buffer[b->readpos];
    b->readpos = mov(b->readpos);
    pthread_cond_signal(&(b->notfull));
    pthread_mutex_unlock(&(b->lock));
    return p;
}

/**
 * 写入的块的指针bp
 * 文件描述符handle
 * 对齐字节大小align
 * 块的大小blockSize
 */
const int CHAR_SIZE = 16; // 一个向量有16个字节
const int BLOCK_SIZE = 256 * 1024;

void *producer(void *arg)
{
    const char *filepath = (const char *)arg;
    int handle = open(filepath, O_RDONLY);
    if (handle < 0)
    {
        cerr << "open file failed\n";
        exit(1);
    }

    while (true)
    {
        char *bp = nullptr;
        int err = posix_memalign((void **)&bp, CHAR_SIZE * sizeof(char), BLOCK_SIZE);
        if (err)
        {
            cerr << "posix_memalign failed\n";
            exit(1);
        }
        int bytes_read = read(handle, bp, BLOCK_SIZE);
        if (bytes_read <= 0)
        {
            put(&buffer, Product(nullptr, -1)); // 读完, 告知消费者已经读完
            free(bp);
            close(handle);
            return nullptr;
        }

        put(&buffer, Product(bp, bytes_read));
        // free(bp);
    }
    close(handle);
    return nullptr;
}
void *consumer(void *arg)
{
    const __m128i endline = _mm_set1_epi8('\n'); // 换行符向量，用于比较
    int cur_len = 0;
    while (true)
    {
        Product p = get(&buffer);
        if (p.bp == nullptr && p.bytes_read == -1) // 已经读完文件
            break;

        char *bp = p.bp;
        int bytes_read = p.bytes_read;

        // 遍历块内的char向量
        for (char *temp = bp; temp < bp + bytes_read; temp += CHAR_SIZE)
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
                vec_offset += ffs;    // 下一行字符在向量内的起始位置
                cur_len += ffs - 1;   // 记录这一行的字符数
                (*(uint32_t *)arg)++; // 行数增加
                if (cur_len >= MAX_LEN)
                    line_num[MAX_LEN - 1]++; // 如果这一行长度超过1024个字符
                else
                    line_num[cur_len]++; // 对应长度的行的个数增加
                r >>= ffs;               // 消除bitmap中已经检索到的换行符
                cur_len = 0;             // 重置行字符数，统计下一行
            }
        }
        free(bp);
    }
    return nullptr;
    // 向量指针，指向检查的向量的起始位置
}
void filelines_pc(const char *filepath, uint32_t *total_line_num)
{
    pthread_t prod_thread, cons_thread;
    init(&buffer);
    pthread_create(&prod_thread, nullptr, producer, (void *)filepath);
    pthread_create(&cons_thread, nullptr, consumer, (void *)total_line_num);
    pthread_join(prod_thread, nullptr);
    pthread_join(cons_thread, nullptr);
}
const char testFile[19] = "test/test.txt";
uint32_t total_line_num;

int main(int argc, char const *argv[])
{
    uint32_t most_freq_len, most_freq_len_linenum;
    total_line_num = 0;
    auto start = high_resolution_clock::now();
    filelines_pc(testFile, &total_line_num);
    find_most_freq_line(line_num, &most_freq_len, &most_freq_len_linenum);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    printf("%d %d %d\n", total_line_num, most_freq_len, most_freq_len_linenum);
    cout << "Time: " << duration.count() << "ms" << endl;
}