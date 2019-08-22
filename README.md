# libredis-client   
[![Build Status](https://travis-ci.org/freeeyes/PSS.svg?branch=master)](https://travis-ci.org/freeeyes/PSS)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0 )
[![GitHub version](https://badge.fury.io/gh/0xsky%2Fxredis.svg)](https://badge.fury.io/gh/0xsky%2Fxredis)
======

C++ Redis client, support redis cluster, connection pool.

**Features:**
* suppert redis cluster, no data move needed
* connection pool
* thread safe
* suport linux and windows

中文版说明文档点[这里](https://github.com/0xsky/libredis-client/blob/master/README-cn.md)

### Dependencies

requires hiredis only

### Install

First step install libhiredis, on a Debian system you can use:

```bash
sudo apt-get install libhiredis-dev
```

on centos/redhat/fedora system you can use:
```bash
sudo yum install hiredis-devel
```

Then checkout the code and compile it
```bash
git clone https://github.com/0xsky/libredis-client
cd libredis-client
make
sudo make install
```

Usage
```C++
#Accessing redis or  redis Cluster 

#include "libredis-client.h"
int main(int argc, char **argv) {
    RedisClient redisclient;
    # Connect to REDIS and establish a connection pool 
    # If this node is a member of the REDIS cluster, 
    # a connection pool is automatically established for each master node.
    redisclient.ConnectRedis("127.0.0.1", 6379, 4);

    RedisResult result;
    redisclient.Command(result, "set %s %s", "key", "hello");
    
    printf("type:%d integer:%lld str:%s \r\n",
        result.type(), result.integer(), result.str());

    return 0;
}
```



<p>Blog: <a href="http://www.0xsky.com/">xSky's Blog</a>
<p>Redis QQ Group: 190107312


