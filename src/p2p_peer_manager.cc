
#include "p2p_peer_manager.h"

#include <thread>
#include <functional>
#include <iostream>
#include <list>
#include <shared_mutex>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include <glog/logging.h>

class P2PPeerManager::Impl{
public:
    Impl(uint16_t port);
    ~Impl();
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
    std::atomic_bool b_run_;
    std::shared_ptr<std::thread> ios_thread_;
    boost::asio::ip::tcp::acceptor acceptor_;
    //===================================
    //peer相关
    //peer操作的锁，只使用一个，避免复杂性
    std::shared_mutex peer_mutex_;
    //刚连接进来的peer
    std::list<std::shared_ptr<P2PPeer>> peer_list_in_tmp_;
    std::list<std::shared_ptr<P2PPeer>> peer_list_out_tmp_;
    //经过注册稳定连接的peer
    std::list<std::shared_ptr<P2PPeer>> peer_list_in_stable_;
    std::list<std::shared_ptr<P2PPeer>> peer_list_out_stable_;
    //脏的或不稳定的peer
    std::list<std::shared_ptr<P2PPeer>> peer_list_in_dirty_;
    std::list<std::shared_ptr<P2PPeer>> peer_list_out_dirty_;
};

P2PPeerManager::Impl::Impl(uint16_t port):
    lession_port_(port),
    acceptor_(ios_, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4(), port)){

}

P2PPeerManager::Impl::~Impl(){
    Stop();
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
    b_run_ = true;
    ios_thread_ = std::make_shared<std::thread>(std::bind(&P2PPeerManager::Impl::ThreadIOS, this));
    return true;
}

bool P2PPeerManager::Impl::StartConnectToPeer(){
    return true;
}

void P2PPeerManager::Impl::Stop(){
    peer_list_in_tmp_.clear();
    peer_list_out_tmp_.clear();
    peer_list_in_stable_.clear();
    peer_list_out_stable_.clear();
    peer_list_in_dirty_.clear();
    peer_list_out_dirty_.clear();
    if(ios_thread_){
        b_run_ = false;
        ios_thread_->join();
    }
    return;
}

std::shared_ptr<P2PPeer> P2PPeerManager::Impl::CreateP2PPeer(){
    return std::make_shared<P2PPeer>(ios_);
}

void P2PPeerManager::Impl::ThreadIOS(){
    while(b_run_){
        ios_.run_one();
        boost::this_thread::yield();
    }
}

void P2PPeerManager::Impl::OnAccept(const boost::system::error_code& error, std::shared_ptr<boost::asio::ip::tcp::socket> socket){
    if(error){
        LOG(ERROR)<<"accept err:"<<error.message();
        return;
    }
    {
        std::shared_lock<std::shared_mutex> lk(peer_mutex_);
        peer_list_in_tmp_.push_back(std::make_shared<P2PPeer>(ios_, socket));
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