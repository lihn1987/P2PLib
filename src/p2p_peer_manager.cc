
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
    void ConnectTo(const boost::asio::ip::tcp::endpoint& ep);
public:
    boost::signals2::connection AddOnPeerConnectInCallback(std::function<void(std::shared_ptr<P2PPeer>)> cb);
    boost::signals2::connection AddOnPeerConnectOutCallback(std::function<void(std::shared_ptr<P2PPeer>)> cb);
    boost::signals2::connection AddOnPeerDisConnectCallback(std::function<void(std::shared_ptr<P2PPeer>)> cb);
    boost::signals2::connection AddOnPeerReceiveMessageCallback(std::function<void(std::shared_ptr<P2PPeer> , const std::string&)> cb);
public:
    std::list<std::shared_ptr<P2PPeer>> PeerListInTmp();
    std::list<std::shared_ptr<P2PPeer>> PeerListOutTmp();

    std::list<std::shared_ptr<P2PPeer>> PeerListInStable();
    std::list<std::shared_ptr<P2PPeer>> PeerListOutStable();

    std::list<std::shared_ptr<P2PPeer>> PeerListInDirty();
    std::list<std::shared_ptr<P2PPeer>> PeerListOutDirty();
private:
    bool StartServer();
    bool StartConnectToPeer();
private:
    void OnPeerAccept(const boost::system::error_code& error, std::shared_ptr<boost::asio::ip::tcp::socket> peer);
    void OnPeerConnect(std::shared_ptr<P2PPeer> peer);
    void OnPeerDisConnect(std::shared_ptr<P2PPeer> peer);
    void OnPeerReceiveMessage(std::shared_ptr<P2PPeer> peer, const std::string& message);
private:
    //io_server的处理线程
    void ThreadIOS();

    
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
    //回调相关
    boost::signals2::signal<void(std::shared_ptr<P2PPeer> peer)> on_peer_connect_in_cb_;
    boost::signals2::signal<void(std::shared_ptr<P2PPeer> peer)> on_peer_connect_out_cb_;
    boost::signals2::signal<void(std::shared_ptr<P2PPeer> peer)> on_peer_disconnect_cb_;
    boost::signals2::signal<void(std::shared_ptr<P2PPeer> peer, const std::string& message)> on_peer_receive_cb_;
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

boost::signals2::connection P2PPeerManager::Impl::AddOnPeerConnectInCallback(std::function<void(std::shared_ptr<P2PPeer>)> cb){
    return on_peer_connect_in_cb_.connect(cb);
}

boost::signals2::connection P2PPeerManager::Impl::AddOnPeerConnectOutCallback(std::function<void(std::shared_ptr<P2PPeer>)> cb){
    return on_peer_connect_out_cb_.connect(cb);
}

boost::signals2::connection P2PPeerManager::Impl::AddOnPeerDisConnectCallback(std::function<void(std::shared_ptr<P2PPeer>)> cb){
    return on_peer_disconnect_cb_.connect(cb);
}

boost::signals2::connection P2PPeerManager::Impl::AddOnPeerReceiveMessageCallback(std::function<void(std::shared_ptr<P2PPeer>, const std::string&)> cb){
    return on_peer_receive_cb_.connect(cb);
}

std::list<std::shared_ptr<P2PPeer>> P2PPeerManager::Impl::PeerListInTmp(){
    std::shared_lock<std::shared_mutex> lk(peer_mutex_);
    return peer_list_in_tmp_;
}

std::list<std::shared_ptr<P2PPeer>> P2PPeerManager::Impl::PeerListOutTmp(){
    std::shared_lock<std::shared_mutex> lk(peer_mutex_);
    return peer_list_out_tmp_;
}

std::list<std::shared_ptr<P2PPeer>> P2PPeerManager::Impl::PeerListInStable(){
    std::shared_lock<std::shared_mutex> lk(peer_mutex_);
    return peer_list_in_stable_;
}

std::list<std::shared_ptr<P2PPeer>> P2PPeerManager::Impl::PeerListOutStable(){
    std::shared_lock<std::shared_mutex> lk(peer_mutex_);
    return peer_list_out_stable_;
}

std::list<std::shared_ptr<P2PPeer>> P2PPeerManager::Impl::PeerListInDirty(){
    std::shared_lock<std::shared_mutex> lk(peer_mutex_);
    return peer_list_in_dirty_;
}

std::list<std::shared_ptr<P2PPeer>> P2PPeerManager::Impl::PeerListOutDirty(){
    std::shared_lock<std::shared_mutex> lk(peer_mutex_);
    return peer_list_out_dirty_;
}

bool P2PPeerManager::Impl::StartServer(){
    Stop();
    //启动ioserver线程
    auto socket = std::shared_ptr<boost::asio::ip::tcp::socket>( new boost::asio::ip::tcp::socket(ios_));
    acceptor_.async_accept(*socket, std::bind(&P2PPeerManager::Impl::OnPeerAccept, this, std::placeholders::_1, socket));
    b_run_ = true;
    ios_thread_ = std::make_shared<std::thread>(std::bind(&P2PPeerManager::Impl::ThreadIOS, this));
    return true;
}

bool P2PPeerManager::Impl::StartConnectToPeer(){
    return true;
}

void P2PPeerManager::Impl::OnPeerAccept(const boost::system::error_code& error, std::shared_ptr<boost::asio::ip::tcp::socket> socket){
    if(error){
        LOG(ERROR)<<"accept err:"<<error.message();
        return;
    }
    {
        std::shared_lock<std::shared_mutex> lk(peer_mutex_);
        std::shared_ptr<P2PPeer> peer = std::make_shared<P2PPeer>(ios_, socket);
        peer_list_in_tmp_.push_back(peer);
        on_peer_connect_in_cb_(peer);
    }
    LOG(WARNING)<<"接收到了新的连接:("<<socket->remote_endpoint().address().to_string()<<":"<<socket->remote_endpoint().port()<<")=>("
                                    <<socket->local_endpoint().address().to_string()<<":"<<socket->local_endpoint().port()<<")";

    auto socket_tmp = std::shared_ptr<boost::asio::ip::tcp::socket>( new boost::asio::ip::tcp::socket(ios_));
    acceptor_.async_accept(*socket_tmp, boost::bind(&P2PPeerManager::Impl::OnPeerAccept, this, _1, socket_tmp));
}

void P2PPeerManager::Impl::OnPeerConnect(std::shared_ptr<P2PPeer> peer){
    std::lock_guard<std::shared_mutex> lk(peer_mutex_);
    peer_list_out_tmp_.push_back(peer);
    on_peer_connect_out_cb_(peer);
}

void P2PPeerManager::Impl::OnPeerDisConnect(std::shared_ptr<P2PPeer> peer){
    std::lock_guard<std::shared_mutex> lk(peer_mutex_);
    peer_list_in_tmp_.remove(peer);
    peer_list_out_tmp_.remove(peer);
    peer_list_in_stable_.remove(peer);
    peer_list_out_stable_.remove(peer);
    peer_list_in_dirty_.remove(peer);
    peer_list_out_dirty_.remove(peer);
    on_peer_disconnect_cb_(peer);
}

void P2PPeerManager::Impl::OnPeerReceiveMessage(std::shared_ptr<P2PPeer> peer, const std::string &message){
    on_peer_receive_cb_(peer, message);
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

void P2PPeerManager::Impl::ConnectTo(const boost::asio::ip::tcp::endpoint& ep){
    auto peer = std::make_shared<P2PPeer>(ios_);
    peer->ConnectTo(ep);
    peer->AddOnConnectCallback(std::bind(&P2PPeerManager::Impl::OnPeerConnect, this, peer));
    peer->AddOnErrorCallback(std::bind(&P2PPeerManager::Impl::OnPeerConnect, this, peer));
    peer->AddOnReceiveCallback(std::bind(&P2PPeerManager::Impl::OnPeerReceiveMessage, this, peer, std::placeholders::_1));
}

void P2PPeerManager::Impl::ThreadIOS(){
    while(b_run_){
        ios_.run_one();
        boost::this_thread::yield();
    }
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

void P2PPeerManager::ConnectTo(const boost::asio::ip::tcp::endpoint& ep){
  return impl_->ConnectTo(ep);
}

boost::signals2::connection P2PPeerManager::AddOnPeerConnectInCallback(std::function<void(std::shared_ptr<P2PPeer>)> cb){
    return impl_->AddOnPeerConnectInCallback(cb);
}

boost::signals2::connection P2PPeerManager::AddOnPeerConnectOutCallback(std::function<void(std::shared_ptr<P2PPeer>)> cb){
    return impl_->AddOnPeerConnectOutCallback(cb);
}

boost::signals2::connection P2PPeerManager::AddOnPeerDisConnectCallback(std::function<void(std::shared_ptr<P2PPeer>)> cb){
    return impl_->AddOnPeerDisConnectCallback(cb);
}

boost::signals2::connection P2PPeerManager::AddOnPeerReceiveMessageCallback(std::function<void(std::shared_ptr<P2PPeer>, const std::string&)> cb){
    return impl_->AddOnPeerReceiveMessageCallback(cb);
}

std::list<std::shared_ptr<P2PPeer>> P2PPeerManager::PeerListInTmp(){
    return impl_->PeerListInTmp();
}

std::list<std::shared_ptr<P2PPeer>> P2PPeerManager::PeerListOutTmp(){
    return impl_->PeerListOutTmp();
}

std::list<std::shared_ptr<P2PPeer>> P2PPeerManager::PeerListInStable(){
    return impl_->PeerListInStable();
}

std::list<std::shared_ptr<P2PPeer>> P2PPeerManager::PeerListOutStable(){
    return impl_->PeerListOutStable();
}

std::list<std::shared_ptr<P2PPeer>> P2PPeerManager::PeerListInDirty(){
    return impl_->PeerListInDirty();
}

std::list<std::shared_ptr<P2PPeer>> P2PPeerManager::PeerListOutDirty(){
    return impl_->PeerListOutDirty();
}

