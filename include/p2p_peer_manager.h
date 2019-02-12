#ifndef __P2P_PEER_MANAGER__H__
#define __P2P_PEER_MANAGER__H__

#include <stdint.h>
#include <memory>

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
private: 
    class Impl;
    std::shared_ptr<Impl> impl_;
};

#endif