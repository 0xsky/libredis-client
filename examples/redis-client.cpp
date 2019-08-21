#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
using std::cout;
using std::endl;
using std::cin;

#include "libredis-client.h"

static int str2Vect(const char* pSrc, vector<string> &vDest, const char *pSep = ",") {
    if (NULL == pSrc) {
        return -1;
    }

    int iLen = strlen(pSrc);
    if (iLen == 0) {
        return -1;
    }

    char *pTmp = new char[iLen + 1];
    if (pTmp == NULL) {
        return -1;
    }

    memcpy(pTmp, pSrc, iLen);
    pTmp[iLen] = '\0';

    char *pCurr = strtok(pTmp, pSep);
    while (pCurr) {
        vDest.push_back(pCurr);
        pCurr = strtok(NULL, pSep);
    }

    delete[] pTmp;
    return 0;
}

int main(int argc, char **argv) {

    if (2!= argc){
        printf("%s host port", argv[0]);
        return -1;
    }

    RedisClient redisclient;
    bool bRet = redisclient.ConnectRedis(argv[1], atoi(argv[2]), 4);
    if(!bRet) {
        fprintf(stderr, "Connect to %s:%s error\r\n", argv[0], argv[1]);
        return -1;
    }

    std::string  strInput;
    while (true) {
        cout << "\033[32mxRedis> \033[0m";
        getline(cin, strInput);
        if (!cin)
            return 0;

        if (strInput.length() < 1) {
            cout << "input again" << endl;
        }
        else {
            RedisResult result;
            VSTRING vDataIn;

            str2Vect(strInput.c_str(), vDataIn, " ");
            redisclient.RedisCommandArgv(vDataIn, result);

            switch (result.type()){
            case REDIS_REPLY_INTEGER:{ printf("%lld \r\n", result.integer()); break; }
            case REDIS_REPLY_NIL:    { printf("%lld %s \r\n", result.integer(), result.str()); break; }
            case REDIS_REPLY_STATUS: { printf("%s \r\n", result.str()); break; }
            case REDIS_REPLY_ERROR:  { printf("%s \r\n", result.str()); break; }
            case REDIS_REPLY_STRING: { printf("%s \r\n", result.str()); break; }
            case REDIS_REPLY_ARRAY:  {
                for (size_t i = 0; i < result.elements(); ++i) {
                    RedisResult::RedisReply reply = result.element(i);
                    printf("type:%d integer:%lld str:%s \r\n",
                        reply.type(), reply.integer(), reply.str());
                }
                break;
            }
            }
        }
    }

    
    return 0;
}












