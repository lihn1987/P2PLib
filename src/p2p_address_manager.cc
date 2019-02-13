
#include "p2p_address_manager.h"

bool P2PAdressManager::AddWhiteAddr(const boost::asio::ip::tcp::endpoint& addr){
    if(white_addr_list_.find(addr) != white_addr_list_.end()){
        return false;
    }else{
        white_addr_list_.insert(addr);
    }
    return  true;
}

bool P2PAdressManager::RemoveWhiteAddr(const boost::asio::ip::tcp::endpoint& addr){
    if(white_addr_list_.find(addr) == white_addr_list_.end()){
        return false;
    }else{
        white_addr_list_.erase(addr);
    }
    return true;
}

void P2PAdressManager::ResetWhiteAddr(){
    white_addr_list_.clear();
}

bool P2PAdressManager::IsInWhiteAddr(const boost::asio::ip::tcp::endpoint& addr){
    if(white_addr_list_.find(addr) != white_addr_list_.end()){
        return false;
    }
    return true;
}

bool P2PAdressManager::AddBlackAddr(const boost::asio::ip::tcp::endpoint& addr){
    if(black_addr_list_.find(addr) != black_addr_list_.end()){
        return false;
    }else{
        black_addr_list_.insert(addr);
    }
    return true;
}

bool P2PAdressManager::RemoveBlackAddr(const boost::asio::ip::tcp::endpoint& addr){
    if(black_addr_list_.find(addr) == black_addr_list_.end()){
        return false;
    }else{
        black_addr_list_.erase(addr);
    }
    return true;
}

void P2PAdressManager::ResetBlackAddr(){
    black_addr_list_.clear();
}

bool P2PAdressManager::IsInBlackAddr(const boost::asio::ip::tcp::endpoint& addr){
    if(black_addr_list_.find(addr) != black_addr_list_.end()){
        return false;
    }
    return true;
}

bool P2PAdressManager::Load(){
    return true;
}

bool P2PAdressManager::Save(){
    return false;
}
