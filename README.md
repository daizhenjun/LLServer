LLServer
========

A nosql database base on libevent,leveldb.support memcached protocal.

LLServer 1.0 is a high-performance, distributed string store system.it support http , memcached protocal, base on libevent, leveldb project. 

LLServer是本人基于libevent和leveldb这两个开源软件，开发的轻量级数据存储服务器软件，借助libevent高效网络接口实现对leveldb的访问封装。 

项目网址：http://code.google.com/p/llserver/ 　 

使用环境：Linux 　 

作者：代震军 　 

目前发布版本：1.0 


　 支持http协议和memcached协议。也就是可以通过浏览器或现有的memcached客户端来进行数据的CURD操作。 

下面简单介绍一下如何安装使用LLServer。 

LLServer 编译安装： 

1.安装libevent2.0 
=============================================
ulimit -SHn 65535 wget http://monkey.org/~provos/libevent-2.0.12-stable.tar.gz 

tar zxvf libevent-2.0.12-stable.tar.gz 

cd libevent-2.0.12-stable/ 

./configure --prefix=/usr 

make && make install 

cd ../ 

2.通过svn:客户端下载leveldb到本地leveldb文件夹
=============================================
链接：http://leveldb.googlecode.com/svn/trunk/ 或暂时用我这个打好包的地址下载： 

wget http://llserver.googlecode.com/files/leveldb.tar.gz 

tar zxvf leveldb.tar.gz 

之后编译安装 cd leveldb/ 

make -f Makefile 

cp libleveldb.a /usr/local/lib/ 

cp -rf include/ /usr/local/include/ 

cd ../ 

3.LLServer下载地址：
==============================================
wget http://llserver.googlecode.com/files/llserver-1.0.tar.gz 

tar zxvf llserver-1.0.tar.gz 

cd llserver/ 

make -f Makefile 

make install 

cd ../ 

4.LLServer 使用文档\
==============================================
[root@~]# llserver -h -l 

<ip_addr>监听的IP地址，默认值为 0.0.0.0 -p 

<num>监听的TCP端口（默认值：11211） -x 

<path>数据库目录，目录不存在会自动创建（例如：/llserver/data） -c 数据缓存队列单位，默认为100m -t 

<second>HTTP请求的超时时间 -s 1:http协议 other:memcached协议 -d 以守护进程运行 -h 显示帮助 更多内容参见： 


http://www.cnblogs.com/daizhj/archive/2011/08/23/2150422.html 

http://www.cnblogs.com/daizhj/ 

http://weibo.com/daizhj 