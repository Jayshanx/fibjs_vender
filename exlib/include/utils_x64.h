/*
 *  utils_x64.h
 *  Created on: Jul 17, 2014
 *
 *  Copyright (c) 2014 by Leo Hoo
 *  lion@9465.net
 */

#ifdef Darwin
#include <libkern/OSAtomic.h>
#endif

namespace exlib {

#pragma pack(1)

class registers {
public:
    union {
        intptr_t Rbp;
        intptr_t fp;
    };
    intptr_t Rbx;
    intptr_t Rcx;
    intptr_t Rdx;
    intptr_t Rsi;
    intptr_t Rdi;
    intptr_t R8;
    intptr_t R9;
    intptr_t R10;
    intptr_t R11;
    intptr_t R12;
    intptr_t R13;
    intptr_t R14;
    intptr_t R15;
    union {
        intptr_t Rsp;
        intptr_t sp;
    };
};

#pragma pack()

#if defined(Darwin)
inline void MemoryBarrier()
{
    OSMemoryBarrier();
}
#else
inline void MemoryBarrier()
{
    __asm__ volatile(""
                     :
                     :
                     : "memory");
}
#endif

inline void yield()
{
    __asm__ volatile("pause");
}

template <typename T>
inline T* CompareAndSwap(T* volatile* ptr, T* old_value, T* new_value)
{
    T* prev;
    __asm__ volatile(
        "lock; cmpxchgq %1,%2"
        : "=a"(prev)
        : "q"(new_value), "m"(*ptr), "0"(old_value)
        : "memory");
    return prev;
}

inline intptr_t CompareAndSwap(volatile intptr_t* ptr, intptr_t old_value, intptr_t new_value)
{
    intptr_t prev;
    __asm__ volatile(
        "lock; cmpxchgq %1,%2"
        : "=a"(prev)
        : "q"(new_value), "m"(*ptr), "0"(old_value)
        : "memory");
    return prev;
}

inline intptr_t atom_add(volatile intptr_t* dest, intptr_t incr)
{
    intptr_t ret;
    __asm__ volatile(
        "lock; xaddq %0,(%1)"
        : "=r"(ret)
        : "r"(dest), "0"(incr)
        : "memory");
    return ret + incr;
}

inline intptr_t atom_xchg(volatile intptr_t* ptr, intptr_t new_value)
{
    intptr_t prev;

    __asm__ volatile(
        "lock xchgq %2,(%1)"
        : "=r"(prev)
        : "r"(ptr), "0"(new_value)
        : "memory");

    return prev;
}

inline intptr_t atom_inc(volatile intptr_t* dest)
{
    return atom_add(dest, 1);
}

inline intptr_t atom_dec(volatile intptr_t* dest)
{
    return atom_add(dest, -1);
}

inline int32_t CompareAndSwap(volatile int32_t* ptr, int32_t old_value, int32_t new_value)
{
    int32_t prev;
    __asm__ volatile(
        "lock; cmpxchgl %1,%2"
        : "=a"(prev)
        : "q"(new_value), "m"(*ptr), "0"(old_value)
        : "memory");
    return prev;
}

inline int32_t atom_add(volatile int32_t* dest, int32_t incr)
{
    int32_t ret;
    __asm__ volatile(
        "lock; xaddl %0,(%1)"
        : "=r"(ret)
        : "r"(dest), "0"(incr)
        : "memory");
    return ret + incr;
}

inline int32_t atom_xchg(volatile int32_t* ptr, int32_t new_value)
{
    int32_t prev;

    __asm__ volatile(
        "lock xchgl %2,(%1)"
        : "=r"(prev)
        : "r"(ptr), "0"(new_value)
        : "memory");

    return prev;
}

inline int32_t atom_inc(volatile int32_t* dest)
{
    return atom_add(dest, 1);
}

inline int32_t atom_dec(volatile int32_t* dest)
{
    return atom_add(dest, -1);
}

template <typename T>
inline T* atom_xchg(T* volatile* ptr, T* new_value)
{
    T* prev;

    __asm__ volatile(
        "lock xchgq %2,(%1)"
        : "=r"(prev)
        : "r"(ptr), "0"(new_value)
        : "memory");

    return prev;
}

inline void* CompareAndSwap(void* volatile* ptr, void* old_value, void* new_value)
{
    return CompareAndSwap((char* volatile*)ptr, (char*)old_value, (char*)new_value);
}
}
