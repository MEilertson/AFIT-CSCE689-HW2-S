#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <list>
#include <memory>
#include "Server.h"
#include "FileDesc.h"
#include "TCPConn.h"
#include "LogMgr.h"

class TCPServer : public Server 
{
public:
   TCPServer(const char *log_file);
   ~TCPServer();

   void bindSvr(const char *ip_addr, unsigned short port);
   void listenSvr();
   void shutdown();

private:
   // Class to manage the server socket
   SocketFD _sockfd;

   LogMgr _logger;
 
   // List of TCPConn objects to manage connections
   std::list<std::unique_ptr<TCPConn>> _connlist;

   //Logger
   std::string _server_log;
   std::vector<std::string> white_list_IPs;

};


#endif
