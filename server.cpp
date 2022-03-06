//
//  server.cpp
//  netApp
//
//  Created by Jan Cakl on 03.03.2022.
//

#include "server.hpp"
#include <unistd.h>
#include "retTypes.hpp"
#include <queue>
#include <thread>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <regex>
#include "graber.hpp"

#define TRUE 1
#define FALSE 0

#define SERVER_PORT  12345
#define MAX_QUEUE_SIZE 100

template <typename T>
class blocking_bounded_queue {
public:
    blocking_bounded_queue(size_t max_size = MAX_QUEUE_SIZE):
        max_size(max_size)
    {}
   // add url adress to the queue, check closing and max size of queue
    bool add(T const& e) {
        std::unique_lock<std::mutex> lg(mutex);
        addVar.wait(lg, [&] () -> bool {
            return queue.size() < max_size || closing;
            });
        
        if (closing) {
            return false;
        }
        queue.push(e);
        takeVar.notify_one();
        return true;
    }

    // get url from queue
    std::pair<bool, T> take() {
        std::unique_lock<std::mutex> lg(mutex);
        takeVar.wait(lg, [&] () -> bool {
            return queue.size() > 0 || closing;
        });
        
        if (queue.size() == 0) {
            return { false, T{} };
        }
        
        auto out = std::move(queue.front()); queue.pop();
        addVar.notify_one();
        return { true, std::move(out) };
    }
    // close queue
    void close() {
        std::unique_lock<std::mutex> lg(mutex);
        closing = true;
        takeVar.notify_all();
        addVar.notify_all();
    }

private:
    std::queue<T> queue;
    std::mutex mutex;
    std::condition_variable takeVar;
    std::condition_variable addVar;
    size_t max_size;
    bool closing = false;
};

int serverApp(){
    using namespace std::chrono_literals;
    
    blocking_bounded_queue<std::string> queue;
    
    auto producer = [] (blocking_bounded_queue<std::string>& queue) {
    
       int i = 1;
       int len = 1;
       int rc = 1;
       int on = 1;
       int listenSd, maxSd, newSd;
       int descReady, endServer = FALSE;
       int close_conn;
       char buffer[80];
       struct sockaddr_in6 addr;
       struct timeval timeout;
       fd_set masterSet, workingSet;

       // Create AF_INET6 socket to receive incoming connection
       listenSd = socket(AF_INET6, SOCK_STREAM, 0);
       if (listenSd < 0){
          std::cout << "Create AF_INET6 socket failed" << std::endl;
          exit(-1);
       }

       // Allow socket descriptor to be reuseable
       rc = setsockopt(listenSd, SOL_SOCKET,  SO_REUSEADDR,
                       (char *)&on, sizeof(on));
       if (rc < 0){
          std::cout << "Socket descriptor reuseable failed" << std::endl;
          close(listenSd);
          exit(-1);
       }

       // Nonblocking socket - other sockets are also nonblocking (inherit from listening sock)
       rc = ioctl(listenSd, FIONBIO, (char *)&on);
       if (rc < 0){
          std::cout << "Nonblocking socket failed (ioctl())" << std::endl;
          close(listenSd);
          exit(-1);
       }

       // Socket binding
       memset(&addr, 0, sizeof(addr));
       addr.sin6_family = AF_INET6;
       memcpy(&addr.sin6_addr, &in6addr_any, sizeof(in6addr_any));
       addr.sin6_port = htons(SERVER_PORT);
       rc = bind(listenSd, (struct sockaddr *)&addr, sizeof(addr));
       
       if (rc < 0){
          std::cout << "Binding failed" << std::endl;
          close(listenSd);
          exit(-1);
       }

       // Listening...
       rc = listen(listenSd, 32);
       if (rc < 0){
          std::cout << "Listen failed" << std::endl;
          close(listenSd);
          exit(-1);
       }

       // Init master fd_set
       FD_ZERO(&masterSet);
       maxSd = listenSd;
       FD_SET(listenSd, &masterSet);

       // Timeout set - 10 minutes
       timeout.tv_sec  = 10 * 60;
       timeout.tv_usec = 0;

       // Loop waiting for inc. connects / data
       do
       {
          // Copy the master fd_set over to the working fd_set.
          memcpy(&workingSet, &masterSet, sizeof(masterSet));

          // Call select() and wait 10 minutes for it to complete.
          std::cout << "I am waiting for the message..." << std::endl;
           
          rc = select(maxSd + 1, &workingSet, NULL, NULL, &timeout);

          // Check if select failed.
          if (rc < 0){
             std::cout << "select() failed" << std::endl;
             break;
          }

          // Check 10 minute timeout.
          if (rc == 0){
             std::cout << "Timed out. End webgrab.." << std::endl;
             break;
          }

          // Descriptors are readable. Determine it..
          descReady = rc;
          for (i=0; i <= maxSd  &&  descReady > 0; ++i)
          {
             //Check if desc is ready
             if (FD_ISSET(i, &workingSet))
             {
                 descReady -= 1;

                // Check listening socket
                if (i == listenSd)
                {
                   //std::cout << "Listening socket is readable" << std::endl;
                   // Accept all incoming connections that are
                   // queued up on the listening socket before we
                   // loop back and call select again.
                   do
                   {
                      // Accept each incoming connection.
                      // If accept fails with EWOULDBLOCK, accepted all of them
                      // Any other failure on accept will cause us to end the server.
                      newSd = accept(listenSd, NULL, NULL);
                      
                      if (newSd < 0){
                         if (errno != EWOULDBLOCK){
                             std::cout << "Accepting failed.." << std::endl;
                             endServer = TRUE;
                         }
                         break;
                      }

                      // New incoming connection
                      std::cout << "New connection - " << newSd << std::endl;
                      FD_SET(newSd, &masterSet);
                      if (newSd > maxSd)
                          maxSd = newSd;
                   } while (newSd != -1);
                }else{
                   // not listening socket, must readable
                   //std::cout << "Descriptor - " << i << "is readable" << std::endl;

                   close_conn = FALSE;
                   
                   do{
                      // Receive data..
                      rc = recv(i, buffer, sizeof(buffer) - 1, 0);
                      buffer[rc] = 0;
                       
                      if (rc < 0){
                         if (errno != EWOULDBLOCK){
                            std::cout << "Receive data failed.." << std::endl;
                            close_conn = TRUE;
                         }
                         break;
                      }

                      // Check if connection closed by client
                      if (rc == 0){
                         std::cout << "Connection closed" << std::endl;
                         close_conn = TRUE;
                         break;
                      }

                      // Data received
                      len = rc;
                
                      std::cout << len << "bytes received" <<std::endl;
                       
                      if( strcmp(buffer, "s") == 0 ){
                          std::cout << "Exit daemon" << std::endl;
                          queue.close();
                          endServer = TRUE;
                      }else if( buffer[0] == 'd' ){
                          std::string str(buffer + 2);
                          queue.add(str);
                      }
                      // Send the same data to the client
                      rc = send(i, buffer, len, 0);
                      if (rc < 0){
                         std::cout << "Send failed" << std::endl;
                         close_conn = TRUE;
                         break;
                      }
                   } while (TRUE);

                   
                   // if flag close_conn is active, clean up active connection
                   if (close_conn){
                      close(i);
                      FD_CLR(i, &masterSet);
                      if (i == maxSd){
                         while (FD_ISSET(maxSd, &masterSet) == FALSE)
                             maxSd -= 1;
                      }
                   }
                }
             }
          }
       }while (endServer == FALSE);

       // Clean up all open sockets
       for (i=0; i <= maxSd; ++i){
          if (FD_ISSET(i, &masterSet))
             close(i);
       }
    };
    
    auto consumer = [] (blocking_bounded_queue<std::string>& queue) {
        while (true) {
            auto elem = queue.take();
            if (!elem.first) {
                break;
            }

            if(grabUrl(elem.second)){
                std::cout << "Grab data are ok" << std::endl;
            }else{
                std::cout << "Grab data are not ok" << std::endl;
            }
            std::this_thread::sleep_for(100ms);
        }
    };
    
    std::thread t1(producer, std::ref(queue));
    std::thread t2(consumer, std::ref(queue));
    std::thread t3(consumer, std::ref(queue));
    
    t1.join();
    t2.join();
    t3.join();
    
    return EXIT_SUCCESS;
};

