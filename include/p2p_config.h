#ifndef __P2P__CONFIG__H__2018_12_29__
#define __P2P__CONFIG__H__2018_12_29__
class P2PConfig{
public:
    uint32_t max_in_peer_size();
    void set_max_in_peer_size();
    uint32_t max_out_peer_size();
    void set_max_out_peer_size();
    bool accept_nat();
    void set_accept_nat();
public:
    bool Load();
    bool Save();
    bool Reset();
private:
    uint32_t max_in_peer_size_;
    uint32_t max_out_peer_size_;
    bool accept_nat_;
};
#endif
