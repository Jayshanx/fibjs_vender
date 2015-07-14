/*
 *  fiber.h
 *  Created on: Mar 19, 2012
 *
 *  Copyright (c) 2012 by Leo Hoo
 *  lion@9465.net
 */

#ifndef _ex_fiber_h__
#define _ex_fiber_h__

#include "osconfig.h"
#include <stdint.h>
#include <string.h>
#include <string>
#include "stack.h"
#include "list.h"

namespace exlib
{

class Fiber;
class Locker
{
public:
    Locker(bool recursive = true) :
        m_recursive(recursive), m_count(0), m_locker(0)
    {
    }

public:
    void lock();
    void unlock();
    bool trylock();
    bool owned();

    int blocked() const
    {
        return m_blocks.count();
    }

private:
    bool m_recursive;
    int m_count;
    Fiber *m_locker;
    List<Fiber> m_blocks;
#ifdef DEBUG
    trace m_trace;
#endif
};

class autoLocker
{
public:
    autoLocker(Locker &l) :
        m_l(l)
    {
        m_l.lock();
    }

    ~autoLocker()
    {
        m_l.unlock();
    }

private:
    Locker &m_l;
};

class Event
{
public:
    Event()
    {
        m_set = false;
    }

public:
    void wait();
    void pulse();
    void set();
    void reset();
    bool isSet();

private:
    bool m_set;
    List<Fiber> m_blocks;
};

class CondVar
{
public:
    void wait(Locker &l);
    void notify_one();
    void notify_all();

    int blocked() const
    {
        return m_blocks.count();
    }

private:
    List<Fiber> m_blocks;
};

class Semaphore
{
public:
    Semaphore(int count = 0) :
        m_count(count)
    {
    }

public:
    void wait();
    void post();
    bool trywait();

    int blocked() const
    {
        return m_blocks.count();
    }

private:
    int m_count;
    List<Fiber> m_blocks;
};

template<class T>
class Queue
{
public:
    void put(T *pNew)
    {
        m_list.putTail(pNew);
        m_sem.post();
    }

    T *get()
    {
        m_sem.wait();
        return m_list.getHead();
    }

    T *tryget()
    {
        if (!m_sem.trywait())
            return 0;
        return m_list.getHead();
    }

    bool empty() const
    {
        return m_list.empty();
    }

    int count() const
    {
        return m_list.count();
    }

public:
    List<T> m_list;
    Semaphore m_sem;
};

#define TLS_SIZE    8

#define FIBER_STACK_SIZE    (65536 * 2)

class AsyncEvent : public linkitem
{
public:
    virtual ~AsyncEvent()
    {}

    virtual int post(int v);
    virtual int apost(int v)
    {
        return post(v);
    }

    virtual void callback()
    {
        weak.set();
    }

    int wait()
    {
        weak.wait();
        return m_v;
    }

    bool isSet()
    {
        return weak.isSet();
    }

    int result()
    {
        return m_v;
    }

    void sleep(int ms);

private:
    Event weak;
    int m_v;
};

class Fiber : public linkitem
{
public:
    Fiber() : refs_(0)
    {
        memset(&m_cntxt, 0, sizeof(m_cntxt));
        memset(&m_tls, 0, sizeof(m_tls));
        memset(&name_, 0, sizeof(name_));
    }

public:
    void Ref()
    {
        refs_++;
    }

    void Unref()
    {
        if (--refs_ == 0)
            destroy();
    }

    void join()
    {
        m_joins.wait();
    }

    void set_name(const char *name)
    {
        strncpy(name_, name, sizeof(name_));
        name_[sizeof(name_) - 1] = '\0';
    }

    const char* name()
    {
        return name_;
    }

public:
    static void sleep(int ms);
    static Fiber *Current();
    static Fiber *Create(void *(*func)(void *), void *data = 0,
                         int stacksize = FIBER_STACK_SIZE);

    static int tlsAlloc();
    static void *tlsGet(int idx);
    static void tlsPut(int idx, void *v);
    static void tlsFree(int idx);

private:
    void destroy();

public:
    intptr_t refs_;
    context m_cntxt;
    Event m_joins;
    void *m_tls[TLS_SIZE];
    char name_[16];

#ifdef DEBUG
    linkitem m_link;
    trace m_trace;
#endif
};

}

#endif

