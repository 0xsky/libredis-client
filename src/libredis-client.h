/*
* ----------------------------------------------------------------------------
* Copyright (c) 2013-2019, xSky <guozhw at gmail dot com>
* All rights reserved.
* Distributed under GPL license.
* ----------------------------------------------------------------------------
*/

#ifndef _LIB_REDIS_CLIENT_H_
#define _LIB_REDIS_CLIENT_H_

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
}RedisConnection;


typedef std::list <RedisConnection *> RedisConnectionList;
typedef std::list <RedisConnection *>::iterator RedisConnectionIter;

class xLock;
struct NodeInfo;
class RedisClient
{
  public:
    RedisClient();
    ~RedisClient();

public:
    bool ConnectRedis(const char *host, uint32_t port, uint32_t poolsize);
    bool Command(RedisResult &result, const char *format, ...);
    bool CommandArgv(const VSTRING& vDataIn, RedisResult &result);
    void Keepalive();
    void Release();

private:
    
    bool ReConnectRedis(RedisConnection *pConn);
    bool ConnectRedisNode(int idx, const char *host, uint32_t port, uint32_t poolsize);
    bool CheckReply(redisReply *reply);
    uint32_t KeyHashSlot(const char *key, size_t keylen);
    bool ClusterEnabled(redisContext *ctx);
    bool ClusterStatus(redisContext *ctx);
    bool GetClusterNodes(redisContext *redis_ctx);
    RedisConnection *GetConnection(uint32_t idx);
    void FreeConnection(RedisConnection * pRedis);
    uint32_t FindNodeIndex(uint32_t slot);
    uint32_t GetKeySlotIndex(const char* key);
    RedisConnection *FindNodeConnection(const char *key);

private:
    typedef std::vector<NodeInfo> NODELIST;
private:
    RedisConnectionList   *mRedisConnList;
    xLock                 *mLock;         // 锁表
    uint32_t               mPoolSize;
    NODELIST               mClusterNodes;
    bool                   mClusterEnabled;
    
};


#endif
