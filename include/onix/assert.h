#ifndef ONIX_ASSERT_H
#define ONIX_ASSERT_H

// file：文件名字，line：行号
void assertion_failure(char *exp, char *file, char *base, int line);

// assert 判断，当exp不满足的时候就报错,#exp传入当前比较的表达式字符串
#define assert(exp) \
        if(exp)     \
            ;       \
        else        \
            assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)

// 内核错误，报错
void panic(const char *fmt, ...);


#endif