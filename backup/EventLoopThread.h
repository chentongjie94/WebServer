/* @author: chentongjie
 * @date: 
 * @description: EventLoopThread对象所持有的线程担当IO线程，ThreadPool中的
 * 线程用于执行通用任务(如解析HTTP报文) */

#ifndef _EVENTLOOP_THREAD_H_
#define _EVENTLOOP_THREAD_H_

#include <iostream>
#include <string>
#include <functional>
#include "Thread.h"
#include "EventLoop.h"

class EventLoopThread
{
public:
    typedef std::function<void()> ThreadFunction;
    EventLoopThread(std::string n = std::string());
    ~EventLoopThread();

    EventLoop* getLoop();
    void start();
    void threadFunc();//线程真正执行的函数
private:
    Thread thread_;
    EventLoop *loop_;
    pthread_t threadId_;
    std::string name_;
};
#endif


