#ifndef __P2P__H__2018_12_19__
#define __P2P__H__2018_12_19__

#include <memory>
#include <functional>
#include <list>

#include "p2p_config.h"
#include "p2p_peer.h"
#include "p2p_msg_regist.h"
#include "p2p_state.h"

/**
* P2P的主要接口和流程都封装在该类内
*/
class p2p{
public://起止控制
  bool start(std::shared_ptr<P2PConfig> config);
  bool stop();
public://接入控制
  void SetOnConnectIn(std::function<bool(std::shared_ptr<P2PPeer>)> contorller);
public://注册控制
  void SetOnPeerRegistIn(std::function<bool(std::shared_ptr<P2PPeer>, std::shared_ptr<P2PMsgRegist>)> controller);
  void SetOnPeerRegistOut(std::function<bool(std::shared_ptr<P2PMsgRegist>)> controller);
public://消息控制
  void SetOnGetMessage(std::function<bool(std::shared_ptr<P2PPeer>peer, std::shared_ptr<P2PMsg>)>);
  void SendMsg(std::shared_ptr<P2PPeer> peer, std::shared_ptr<P2PMsg> msg);
  void BroadcastMsg(std::shared_ptr<P2PMsg> msg);
public://状态查看类
  P2PState GetState();
  std::list<std::shared_ptr<P2PPeer>> GetAllPeer();
};

#endif
