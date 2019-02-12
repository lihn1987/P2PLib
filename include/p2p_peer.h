#ifndef __P2P__PEER__H__2018_12_26__
#define __P2P__PEER__H__2018_12_26__

#include <boost/asio.hpp>
#include <memory>
class P2PPeer{
public:
    P2PPeer(boost::asio::io_service& ios, std::shared_ptr<boost::asio::ip::tcp::socket> socket=nullptr);
public:
    void ConnectTo(const boost::asio::ip::tcp::endpoint& ep);
public:
    bool IsConnected();
    boost::asio::ip::tcp::endpoint address_local() const;
    boost::asio::ip::tcp::endpoint address_remote() const;
private:
    void OnConnect(const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint& ep_remote);
private:
    std::shared_ptr<boost::asio::ip::tcp::socket> socket_;
    std::function<void(const boost::system::error_code& ec)> on_connect_callback_;
};

#endif
