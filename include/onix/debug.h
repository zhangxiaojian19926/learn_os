#ifndef ONIX_DEBUG_H
#define ONIX_DEBUG_H

void debugk(char *file, int line, const char *fmt, ...);

// bochs magic breakpoint
#define BMB asm volatile("xchgw %bx, %bx")


// 参数：args...
// GCC编译器中的CPP预编译器,是支持这种写法的，args... 比 args,... 的写法可读性较好。
// 含义：应该是DBG_OUTPUT的参数，第一个传给fmt, 第二个开始传给args...可为多个。
// 参数：##args
// ##args的意思，就是把args...中的多个参数，串连起来。
#define DEBUGK(fmt, args...) debugk(__BASE_FILE__, __LINE__, fmt, ##args)

#endif