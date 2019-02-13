#ifndef __P2P_PEER_MANAGER__H__
#define __P2P_PEER_MANAGER__H__

#include <stdint.h>
#include <memory>

#include <boost/signals2.hpp>

#include "p2p_peer.h"
class P2PPeerManager{
public:
    P2PPeerManager(uint16_t port=8710);
public:
    bool Start();
    void Stop();
public:
    //利用manager的ioservice创建一个P2PPeer的对象
    std::shared_ptr<P2PPeer> CreateP2PPeer();
    void ConnectTo(const boost::asio::ip::tcp::endpoint& ep);
public:
    boost::signals2::connection AddOnPeerConnectInCallback(std::function<void(std::shared_ptr<P2PPeer>)> cb);
    boost::signals2::connection AddOnPeerConnectOutCallback(std::function<void(std::shared_ptr<P2PPeer>)> cb);
    boost::signals2::connection AddOnPeerDisConnectCallback(std::function<void(std::shared_ptr<P2PPeer>)> cb);
    boost::signals2::connection AddOnPeerReceiveMessageCallback(std::function<void(std::shared_ptr<P2PPeer> , const std::string&) > cb);
public:
    std::list<std::shared_ptr<P2PPeer>> PeerListInTmp();
    std::list<std::shared_ptr<P2PPeer>> PeerListOutTmp();

    std::list<std::shared_ptr<P2PPeer>> PeerListInStable();
    std::list<std::shared_ptr<P2PPeer>> PeerListOutStable();

    std::list<std::shared_ptr<P2PPeer>> PeerListInDirty();
    std::list<std::shared_ptr<P2PPeer>> PeerListOutDirty();
private: 
    class Impl;
    std::shared_ptr<Impl> impl_;
};

#endif
