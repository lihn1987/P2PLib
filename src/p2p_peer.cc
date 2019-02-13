#include "p2p_peer.h"

#include <glog/logging.h>


P2PPeer::P2PPeer(boost::asio::io_service& ios, std::shared_ptr<boost::asio::ip::tcp::socket> socket):socket_(socket){
    if(!socket){
        socket_ = std::make_shared<boost::asio::ip::tcp::socket>(ios);
    }else{
        socket_ = socket;
        StartReceive();
    }
}

P2PPeer::~P2PPeer(){
    Disconnect();
}

void P2PPeer::ConnectTo(const boost::asio::ip::tcp::endpoint& ep){
    std::function<void(const boost::system::error_code&, const boost::asio::ip::tcp::endpoint&)> OnConnect;
    OnConnect = [=](const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint& ep){
        if(ec){
            LOG(WARNING)<<"连接到("<<ep.address().to_string()<<":"<<ep.port()<<")失败:"<<ec.message();
        }else{
            LOG(WARNING)<<"连接到("<<ep.address().to_string()<<":"<<ep.port()<<")成功!";
        }
        if(on_connect_callback_)on_connect_callback_(ec);
    };
    LOG(WARNING)<<"正在连接到"<<ep.address().to_string()<<":"<<ep.port();
    socket_->async_connect(ep, std::bind(OnConnect, std::placeholders::_1, ep));
}

void P2PPeer::Disconnect(){
    if(IsConnected()){
        try{
            socket_->cancel();
            socket_->close();
        }catch(...){

        }
    }

}
bool P2PPeer::SendMessage(const std::string& msg){
    return SendMessage(msg.data(), msg.length());
}

bool P2PPeer::SendMessage(const char* msg, uint32_t len){
    if(len == 0)return false;
    LOG(INFO)<<"发送消息:"<<std::string(msg, len);
    std::function<void(const boost::system::error_code& , std::size_t) > OnSendMessage;
    OnSendMessage = 
    [=](
        const boost::system::error_code& ec, // Result of operation.
        std::size_t sended           // Number of bytes sent.
    ){
        if(ec){
            LOG(ERROR)<<"发送失败:"<<ec.message();
        }else{
            //发送成功
            std::lock_guard<std::shared_mutex> lk(send_buffer_mutex_);
            send_buffer_.erase(send_buffer_.begin(), send_buffer_.begin()+sended);
            SendMessage(send_buffer_.data(), send_buffer_.size());
        }
    };
    
    if(!IsConnected())return false;
    std::lock_guard<std::shared_mutex> lk(send_buffer_mutex_);
    send_buffer_.insert(send_buffer_.end(), msg, msg+len);
    socket_->async_send(boost::asio::buffer(send_buffer_), OnSendMessage);
    return true;
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

void P2PPeer::StartReceive(){
    auto OnReceive = [=](
        const boost::system::error_code& ec,
        char* buffer,
        std::size_t readed
    ){
        if(ec){
            LOG(INFO)<<"网络断开！";
            delete[] buffer;
            return;
        }
        std::lock_guard<std::shared_mutex> lk(receive_buffer_mutex_);
        receive_buffer_.insert(receive_buffer_.end(), buffer, buffer+readed);
        LOG(INFO)<<"收到消息:"<<std::string(buffer, readed);
        delete[] buffer;
        StartReceive();
    };

    char* buf = new char[1024*1024];
    socket_->async_receive(boost::asio::buffer(buf, 1024*1024), std::bind(OnReceive, std::placeholders::_1, buf, std::placeholders::_2));
}