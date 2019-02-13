#ifndef __P2P__PEER__H__2018_12_26__
#define __P2P__PEER__H__2018_12_26__

#include <boost/asio.hpp>
#include <vector>
#include <memory>
#include <shared_mutex>
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
    std::function<void(const boost::system::error_code& ec)> on_connect_callback_;
};

#endif
