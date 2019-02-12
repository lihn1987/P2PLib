
#include "p2p_peer_manager.h"

#include <thread>
#include <functional>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include <glog/logging.h>

class P2PPeerManager::Impl{
public:
    Impl(uint16_t port);
public:
    bool Start();
    void Stop();
public:
    std::shared_ptr<P2PPeer> CreateP2PPeer();
private:
    bool StartServer();
    bool StartConnectToPeer();
private:
    //io_server的处理线程
    void ThreadIOS();
    //处理接收到的socket
    void OnAccept(const boost::system::error_code& error, std::shared_ptr<boost::asio::ip::tcp::socket> peer);
    
private:
    uint16_t lession_port_;
    boost::asio::io_service ios_;
    std::shared_ptr<std::thread> ios_thread_;
    boost::asio::ip::tcp::acceptor acceptor_;
    
};

P2PPeerManager::Impl::Impl(uint16_t port):
    lession_port_(port),
    acceptor_(ios_, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4(), port)){

}
bool P2PPeerManager::Impl::Start(){
    if(
        StartServer() &&
        StartConnectToPeer()){
        LOG(INFO)<<"P2P开始监听...";
        return true;
    }
    Stop();
    return false;
}

bool P2PPeerManager::Impl::StartServer(){
    Stop();
    //启动ioserver线程
    auto socket = std::shared_ptr<boost::asio::ip::tcp::socket>( new boost::asio::ip::tcp::socket(ios_));
    acceptor_.async_accept(*socket, std::bind(&P2PPeerManager::Impl::OnAccept, this, std::placeholders::_1, socket));
    ios_thread_ = std::make_shared<std::thread>(std::bind(&P2PPeerManager::Impl::ThreadIOS, this));
    return true;
}

bool P2PPeerManager::Impl::StartConnectToPeer(){
    return true;
}

void P2PPeerManager::Impl::Stop(){
    if(ios_thread_){
        ios_thread_->join();
    }
    return;
}

std::shared_ptr<P2PPeer> P2PPeerManager::Impl::CreateP2PPeer(){
    //boost::asio::ip::tcp::socket socket
    return std::make_shared<P2PPeer>(ios_);
}

#include <iostream>
void P2PPeerManager::Impl::ThreadIOS(){
    while(1){
        ios_.run_one();
        boost::this_thread::yield();
        //std::cout<<"."<<std::endl;
    }
}

void P2PPeerManager::Impl::OnAccept(const boost::system::error_code& error, std::shared_ptr<boost::asio::ip::tcp::socket>socket){
    if(error){
        LOG(ERROR)<<"accept err:"<<error.message();
        return;
    }
    LOG(WARNING)<<"接收到了新的连接:("<<socket->remote_endpoint().address().to_string()<<":"<<socket->remote_endpoint().port()<<")=>("
                                    <<socket->local_endpoint().address().to_string()<<":"<<socket->local_endpoint().port()<<")";
    auto socket_tmp = std::shared_ptr<boost::asio::ip::tcp::socket>( new boost::asio::ip::tcp::socket(ios_));
    acceptor_.async_accept(*socket_tmp, boost::bind(&P2PPeerManager::Impl::OnAccept, this, _1, socket_tmp));
}



//====================
//接口转接
P2PPeerManager::P2PPeerManager(uint16_t port){
    impl_ = std::make_shared<Impl>(port);
}

bool P2PPeerManager::Start(){
    return impl_->Start();
}

void P2PPeerManager::Stop(){
    impl_->Stop();  
}

std::shared_ptr<P2PPeer> P2PPeerManager::CreateP2PPeer(){
    return impl_->CreateP2PPeer();
}