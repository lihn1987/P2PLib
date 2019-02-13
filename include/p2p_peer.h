#ifndef __P2P__PEER__H__2018_12_26__
#define __P2P__PEER__H__2018_12_26__

#include <vector>
#include <memory>
#include <shared_mutex>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

class P2PPeer{
public:
    P2PPeer(boost::asio::io_service& ios, std::shared_ptr<boost::asio::ip::tcp::socket> socket=nullptr);
    ~P2PPeer();
public:
    void ConnectTo(const boost::asio::ip::tcp::endpoint& ep);
    void Disconnect();
    bool SendMessage(const std::string& msg);
    bool SendMessage(const char* msg, uint32_t len);
public:
    //设置回调
    boost::signals2::connection AddOnConnectCallback(std::function<void()> cb);
    boost::signals2::connection AddOnErrorCallback(std::function<void(const boost::system::error_code& ec)> cb);
    boost::signals2::connection AddOnReceiveCallback(std::function<void(const std::string& message)> cb);
public:
    bool IsConnected();
    boost::asio::ip::tcp::endpoint address_local() const;
    boost::asio::ip::tcp::endpoint address_remote() const;
private:
    //开始收取数据
    void StartReceive();
private:
    std::shared_ptr<boost::asio::ip::tcp::socket> socket_;
    //发送缓存
    std::shared_mutex send_buffer_mutex_;
    std::vector<char> send_buffer_;
    //接收缓存
    std::shared_mutex receive_buffer_mutex_;
    std::vector<char> receive_buffer_;
    //回调
    boost::signals2::signal<void(const boost::system::error_code& ec)> on_error_sig;
    boost::signals2::signal<void()> on_connect_sig;
    boost::signals2::signal<void(const std::string& message)> on_receive_sig;

};

#endif
