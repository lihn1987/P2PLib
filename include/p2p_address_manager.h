#ifndef __P2P_ADDRESS_MANAGER__H__
#define __P2P_ADDRESS_MANAGER__H__
#include <unordered_set>
#include <memory>
#include <boost/asio.hpp>
//让其能作为hash的key
namespace std{
    template <> 
    struct hash<boost::asio::ip::tcp::endpoint> {
        int operator()(const boost::asio::ip::tcp::endpoint& item) const{
            return std::hash<std::string>()(item.address().to_string())+std::hash<int>()(item.port());
        }
    };
}

class P2PAdressManager{
public:
//白名单相关操作
    /**
     * 若列表中已经包含addr，则返回false
     */
    bool AddWhiteAddr(const boost::asio::ip::tcp::endpoint& addr);
    /**
     * 若列表中没包含addr，则返回false
     */
    bool RemoveWhiteAddr(const boost::asio::ip::tcp::endpoint& addr);
    void ResetWhiteAddr();
    bool IsInWhiteAddr(const boost::asio::ip::tcp::endpoint& addr);
//黑名单相关操作
    bool AddBlackAddr(const boost::asio::ip::tcp::endpoint& addr);
    bool RemoveBlackAddr(const boost::asio::ip::tcp::endpoint& addr);
    void ResetBlackAddr();
    bool IsInBlackAddr(const boost::asio::ip::tcp::endpoint& addr);
private:
    bool Load();
    bool Save();
private:
    std::unordered_set<boost::asio::ip::tcp::endpoint> white_addr_list_;
    std::unordered_set<boost::asio::ip::tcp::endpoint> black_addr_list_;
};


#endif