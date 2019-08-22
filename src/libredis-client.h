#ifndef _XREDIS_HIREDISCPP_H_
#define _XREDIS_HIREDISCPP_H_

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <string>
#include <list>
#include <vector>
#include "hiredis.h"


#define MAX_REDIS_POOLSIZE 64
#define MAX_TIME_OUT       5

typedef std::vector<std::string>    VSTRING;

uint16_t crc16(const char *buf, int len);

bool CheckReply(const redisReply *reply);
void FreeReply(const redisReply *reply);



class RedisResult {
public:
    RedisResult(){}
    ~RedisResult() {  FreeReply(Reply.reply); }
    
public:
    class RedisReply {
    public:
        RedisReply(redisReply *r) { reply = r;}
        RedisReply() { reply = NULL; }
        ~RedisReply() {}
    
        int type() const { return reply->type; }
        long long integer() const {return reply->integer; }
        int len() const { return reply->len; }
        char* str() const { return reply->str; }
        size_t elements() const { return reply->elements; }
        struct RedisReply element(uint32_t index) const { return RedisReply(reply->element[index]); }
        private:
        friend class RedisResult;
        redisReply *reply;
    };
    
    void Init(redisReply *r) { Reply.reply = r; }
    int type() const { return Reply.type(); }
    long long integer() const {return Reply.integer(); }
    int len() const { return Reply.len(); }
    char* str() const { return Reply.str(); }
    size_t elements() const { return Reply.elements(); }
    RedisReply element(uint32_t index) const { return Reply.element(index); }
private:
    RedisReply Reply;
};

typedef struct REDISCONN {
    REDISCONN() {
        mCtx = NULL;
        mHost = NULL;
        mPort = 0;
        mPoolSize = 0;
    }
    ~REDISCONN(){}
    
    redisContext *mCtx;
    const char   *mHost;
    uint32_t      mPort;
    uint32_t      mPoolSize;
    uint32_t      mIndex;
}RedisConn;

typedef std::list <RedisConn *> RedisConnList;
typedef std::list <RedisConn *>::iterator RedisConnIter;

class xLock;
class RedisClient
{
  public:
    RedisClient();
    ~RedisClient();

private:
    struct NodeInfo
    {
        std::string strinfo;
        std::string id;         // The node ID, a 40 characters random string generated when a node is created and never changed again (unless CLUSTER RESET HARD is used)
        std::string ip;         // The node IP
        uint16_t port;          // The node port
        std::string flags;      // A list of comma separated flags: myself, master, slave, fail?, fail, handshake, noaddr, noflags
        bool is_fail;
        bool is_master;         // true if node is master, false if node is salve
        bool is_slave;
        std::string master_id;  // The replication master
        int ping_sent;          // Milliseconds unix time at which the currently active ping was sent, or zero if there are no pending pings
        int pong_recv;          // Milliseconds unix time the last pong was received
        int epoch;              // The configuration epoch (or version) of the current node (or of the current master if the node is a slave). Each time there is a failover, a new, unique, monotonically increasing configuration epoch is created. If multiple nodes claim to serve the same hash slots, the one with higher configuration epoch wins
        bool connected;         // The state of the link used for the node-to-node cluster bus
        std::vector<std::pair<uint32_t, uint32_t> > mSlots; // A hash slot number or range

        bool CheckSlot(uint32_t slotindex)
        {
            std::vector<std::pair<uint32_t, uint32_t> >::iterator iter = mSlots.begin();
            for (; iter != mSlots.end(); ++iter) {
                //printf("check %u [%u, %u]\n", slotindex, iter->first, iter->second);
                if ((slotindex >= iter->first) && (slotindex <= iter->second)) {
                    return true;
                }
            }
            return false;
        }

        bool ParseNodeString(const std::string &nodeString)
        {
            std::string::size_type ColonPos = nodeString.find(':');
            if (ColonPos == std::string::npos) {
                return false;
            } else {
                const std::string port_str = nodeString.substr(ColonPos + 1);
                port = atoi(port_str.c_str());
                ip = nodeString.substr(0, ColonPos);
                return true;
            }
        }

        void ParseSlotString(const std::string &SlotString)
        {
            uint32_t StartSlot = 0;
            uint32_t EndSlot = 0;
            std::string::size_type BarPos = SlotString.find('-');
            if (BarPos == std::string::npos) {
                StartSlot = atoi(SlotString.c_str());
                EndSlot = StartSlot;
            } else {
                const std::string EndSlotStr = SlotString.substr(BarPos + 1);
                EndSlot = atoi(EndSlotStr.c_str());
                StartSlot = atoi(SlotString.substr(0, BarPos).c_str());
            }
            mSlots.push_back(std::make_pair(StartSlot, EndSlot));
        }
    };

    typedef std::vector<NodeInfo> NODELIST;

public:
    void Init();
    
    bool ConnectRedis(const char *host, uint32_t port, uint32_t poolsize);
    bool ReConnectRedis(RedisConn *pConn);
    void Keepalive();
    bool RedisCommandArgv(const VSTRING& vDataIn, RedisResult &result);
    bool RedisCommand(RedisResult &result, const char *format, ...);

private:
    void Release();
    bool ConnectRedisNode(int idx, const char *host, uint32_t port, uint32_t poolsize);
    bool CheckReply(redisReply *reply);
    uint32_t KeyHashSlot(const char *key, size_t keylen);
    bool ClusterEnabled(redisContext *ctx);
    bool Clusterinfo(redisContext *ctx);
    RedisConn *GetConnection(uint32_t idx);
    void FreeConnection(RedisConn * pRedis);
    uint32_t FindNodeIndex(uint32_t slot);
    uint32_t GetKeySlotIndex(const char* key);
    RedisConn *FindNodeConnection(const char *key);

private:
    RedisConnList   *mRedisConnList;
    xLock           *mLock;         // 锁表
    uint32_t         mPoolSize;
    NODELIST         vNodes;
    bool             mClusterEnabled;
    
};


#endif
