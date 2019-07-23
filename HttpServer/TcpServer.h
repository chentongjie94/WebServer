/* author: chentongjie
 * date:
 * TcpServer持有一个EventLoopThreadPool线程池、accept套接字、EventLoop对象、serverChannel事件注册器
 * 作用：负责向所持有的EventLoop事件分发器登记等待连接事件，同时在连接到来时新建一个TcpConnection对象，
 *       并从EventLoopThreadPool线程池中捞出一个IO线程与该TcpConnection对象绑定, 然后往IO线程中推送连
 *       接事件注册任务，将该连接登记到IO线程中, 该连接的读写任务均由该IO线程负责，
 *       // 主线程只负责在连接关闭或连接发生错误时释放连接所占有的资源。(目前由IO线程负责释放资源，后续
 *       打算建立对象池)
 * */
#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include <functional>
#include <memory>
#include <map>
#include <unordered_set>
#include <iostream>
#include <boost/circular_buffer.hpp>
#include "Socket.h"
#include "EventLoopThreadPool.h"
#include "TcpConnection.h"

#define MAXCONNECTION 10000 // 允许的最大连接数

// 前向声明
class Channel;
class EventLoop;
  
class TcpServer
{
public:
    typedef std::shared_ptr<TcpConnection> SP_TcpConnection;
    typedef std::weak_ptr<TcpConnection> WP_TcpConnection;
    typedef std::shared_ptr<Channel> SP_Channel;
    typedef std::function<void(SP_TcpConnection, EventLoop*)> Callback;

    TcpServer(EventLoop* loop, int port, int threadNum = 0, int idleSeconds = 0);
    ~TcpServer();
    // 启动服务器
    void start();
    // 业务函数注册(一般在HttpSever构造函数中注册HttpServer的成员函数)
    void setNewConnCallback(Callback &&cb)
    {
        newConnectionCallback_ = cb; 
    }
private:
    // 连接清理工作
    void connectionCleanUp(int fd);
    // 连接发生错误处理工作
    void onConnectionError();
    // 传输层对连接处理的函数，业务无关
    void onNewConnection();

    // 控制空闲连接是否关闭的条目。在其析构时，若当前连接还没有释放，则关闭连接。
    struct Entry
    {
        explicit Entry(const WP_TcpConnection& wpConn)
        : wpConn_(wpConn)
        { }

        ~Entry()
        {
            SP_TcpConnection spConn = wpConn_.lock();
            if (spConn)
            {
                // 注意：此处将任务推至IO线程中运行
                std::cout << "The connection is more than time, now it is closed by server!" << std::endl;
                spConn->forceClose();
            }
        }
        WP_TcpConnection wpConn_;
    };

    typedef std::shared_ptr<Entry> SP_Entry;
    typedef std::weak_ptr<Entry> WP_Entry;
    typedef std::unordered_set<SP_Entry> Bucket;
    typedef boost::circular_buffer<Bucket> WeakConnectionList;
    // data
    Socket serverSocket_;// TCP服务器的监听套接字
    EventLoop *loop_;// 所绑定的EventLoop
    EventLoopThreadPool eventLoopThreadPool_;
    SP_Channel spServerChannel_;
    std::map<int, SP_TcpConnection> tcpList_;
    int connCount_;
    WeakConnectionList connectionBuckets_;
    // 业务接口函数(上层Http的处理业务回调函数)
    Callback newConnectionCallback_;
    
    void onTime();
    // TCP连接发送信息或者接收数据时，更新时间轮中的数据
    void onMessage(WP_Entry wpEntry);
};

#endif