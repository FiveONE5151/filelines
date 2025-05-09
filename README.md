# 简介
本项目用于对文本文件进行统计。

输入为一个由ASCII构成的文件file。此文件每行的结尾均为0x0A。文件最后一个字符为0x0A。输出为该文件的行数、出现次数最多的一行长度，以及对应的行数。如果多个长度满足的行数均相同，且高于其它长度，则显示所有的最长长度和对应行数。

```
filelines filepath
```
- filelines：程序名
- filepath: 输入文件file的路径

输出如下：
```
linenum most_freq_len most_freq_len_linenum

linenum:文件的总行数
most_freq_len: 出现次数最多的一行长度
most_freq_len_linemum：对应的行数
```

当输入文件为:
```
1
345
123
abcd
abc
abcde
789
```

输出结果为:
```
7 3 4
```
即此文件共7行，最常出现的行长度为3个字符，3个字符的行数共计4行。

# 文件组成

- filelines: baseline
- filelines_avx:  使用AVX512指令
- filelines_pc: 使用AVX512指令+生产者-消费者模型

# 编译
```
make 或 make release - 编译所有程序（优化版本）
make debug - 编译所有程序（调试版本）
make run - 运行 filelines baseline程序
make clean - 清理所有生成的文件
```

# 运行

```
filelines_X filepath
```
