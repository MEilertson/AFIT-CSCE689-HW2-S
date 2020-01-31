#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdexcept>
#include <strings.h>
#include <vector>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <algorithm>
#include "TCPServer.h"

TCPServer::TCPServer(const char* log_file) :_server_log(log_file) {

}


TCPServer::~TCPServer() {

}

/**********************************************************************************************
 * bindSvr - Creates a network socket and sets it nonblocking so we can loop through looking for
 *           data. Then binds it to the ip address and port
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::bindSvr(const char *ip_addr, short unsigned int port) {
   _logger._log_file = _server_log.c_str();

   struct sockaddr_in servaddr;

   // _server_log.writeLog("Server started.");

   // Set the socket to nonblocking
   _sockfd.setNonBlocking();

   // Load the socket information to prep for binding
   _sockfd.bindFD(ip_addr, port);

   // Load whitelist
   FileFD whitelistFD("whitelist");
   if(!whitelistFD.openFile(FileFD::readfd))
      throw pwfile_error("could not open white list file for reading");
   bool eof = false;
   int result = 0;
   std::string IP;
   while(!eof){
      if((result = whitelistFD.readStr(IP)) < 0){
         throw(pwfile_error("failed reading from whitelist"));
      } else if(result == 0){
         eof = true;
      } else {
         std::cout << "IP: " << IP << "\n";
         white_list_IPs.push_back(IP);
      }

   }

   _logger.logMsg("Server Online\n");

 
}

/**********************************************************************************************
 * listenSvr - Performs a loop to look for connections and create TCPConn objects to handle
 *             them. Also loops through the list of connections and handles data received and
 *             sending of data. 
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::listenSvr() {

   bool online = true;
   timespec sleeptime;
   sleeptime.tv_sec = 0;
   sleeptime.tv_nsec = 100000000;
   int num_read = 0;

   // Start the server socket listening
   _sockfd.listenFD(5);

    
   while (online) {
      struct sockaddr_in cliaddr;
      socklen_t len = sizeof(cliaddr);


      if (_sockfd.hasData()) {
         TCPConn *new_conn = new TCPConn();
         if (!new_conn->accept(_sockfd)) {
            // _server_log.strerrLog("Data received on socket but failed to accept.");
            continue;
         }
         std::cout << "***Got a connection***\n";


         // Get their IP Address string to use in logging
         std::string ipaddr_str;
         new_conn->getIPAddrStr(ipaddr_str);

         //check if on whitelist
         if(std::find(white_list_IPs.begin(), white_list_IPs.end(), ipaddr_str) != white_list_IPs.end()) {
            new_conn->sendText("Welcome to the CSCE 689 Server!\n");
            _connlist.push_back(std::unique_ptr<TCPConn>(new_conn));
            _logger.logMsg(((std::string("Whitelisted Connection: ").append(ipaddr_str)).append(std::string("\n")).c_str()));
         } else {
            _logger.logMsg(((std::string("Blocked Non-Whitelisted Connection: ").append(ipaddr_str)).append(std::string("\n")).c_str()));
            new_conn->disconnect();
         }


         // Change this later
         new_conn->startAuthentication();
      }

      // Loop through our connections, handling them
      std::list<std::unique_ptr<TCPConn>>::iterator tptr = _connlist.begin();
      while (tptr != _connlist.end())
      {
         // If the user lost connection
         if (!(*tptr)->isConnected()) {
            // Log it

            // Remove them from the connect list
            tptr = _connlist.erase(tptr);
            std::cout << "Connection disconnected.\n";
            continue;
         }

         // Process any user inputs
         (*tptr)->handleConnection();

         // Increment our iterator
         tptr++;
      }

      // So we're not chewing up CPU cycles unnecessarily
      nanosleep(&sleeptime, NULL);
   } 


   
}


/**********************************************************************************************
 * shutdown - Cleanly closes the socket FD.
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::shutdown() {

   _sockfd.closeFD();
}


