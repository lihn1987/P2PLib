
#include <glog/logging.h>
#include "p2p_peer_manager.h"
#include <thread>
int main(int argc, char** argv){
    //初始化glog
    system("mkdir log");
    google::InitGoogleLogging((const char *)argv[0]);
    FLAGS_stderrthreshold=google::INFO;
    google::SetLogDestination(google::GLOG_INFO, "./log/");

    P2PPeerManager peer_manager;
    peer_manager.Start();
    getchar();
    auto peer = peer_manager.CreateP2PPeer();
    peer->ConnectTo(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 8710));
    getchar();
    peer->SendMessage("hello~~~~~~~~1");
    getchar();
    peer->SendMessage("hello~~~~~~~~2");
    getchar();
    return 0;
}
