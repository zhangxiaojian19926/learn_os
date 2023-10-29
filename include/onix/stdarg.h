#ifndef ONIX_STDARG_H
#define ONIX_STDARG_H

typedef char* va_list;

// 获取下一个可变参数
#define va_arg(ap, t)  (*(t *)((ap += sizeof(char *)) - sizeof(char *)))
#define va_start(ap, v) (ap = (va_list)&v + sizeof(char *))  
#define va_end(ap) (ap = (va_list)0) // 将va_list 置为NULl

#endif