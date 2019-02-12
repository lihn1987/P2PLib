#include "p2p_peer.h"

#include <glog/logging.h>

P2PPeer::P2PPeer(boost::asio::io_service& ios, std::shared_ptr<boost::asio::ip::tcp::socket> socket):socket_(socket){
    if(!socket){
        socket_ = std::make_shared<boost::asio::ip::tcp::socket>(ios);
    }else{
        socket_ = socket;
    }
}

void P2PPeer::ConnectTo(const boost::asio::ip::tcp::endpoint& ep){
    LOG(WARNING)<<"正在连接到"<<ep.address().to_string()<<":"<<ep.port();
    socket_->async_connect(ep, std::bind(&P2PPeer::OnConnect, this, std::placeholders::_1, ep));
}

bool P2PPeer::IsConnected(){
    return socket_->is_open();
}

boost::asio::ip::tcp::endpoint P2PPeer::address_local()const{
    return socket_->local_endpoint();
}

boost::asio::ip::tcp::endpoint P2PPeer::address_remote()const{
    return socket_->remote_endpoint();
}

void P2PPeer::OnConnect(const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint& ep){
    if(ec){
        LOG(WARNING)<<"连接到("<<ep.address().to_string()<<":"<<ep.port()<<")失败:"<<ec.message();
    }else{
        LOG(WARNING)<<"连接到("<<ep.address().to_string()<<":"<<ep.port()<<")成功!";
    }
    if(on_connect_callback_)on_connect_callback_(ec);
}