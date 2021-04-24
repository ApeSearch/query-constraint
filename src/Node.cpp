#include "../include/Node.h"
#include <iostream>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

#define MAXTHREADS 128
#define BLOOMFILTER
#define BUFFERSIZE 8192
#define PORT 6666
#define FRONENDIP inet_addr("35.196.78.129")

using APESEARCH::unique_ptr;


Node::Node() : pool( MAXTHREADS, MAXTHREADS ), listener(PORT){}

Node::~Node(){}

//Use socket accept to get new sockets
//Use fine grained locking to switch out sockets.
void Node::connectionHandler()
{
    std::cerr << "Currently listening for query requests on port: " << PORT << "\n";
    //Since accept will block this is not spinning
    while(true)
    {
        struct sockaddr_in node_addr;
        socklen_t node_len = sizeof(node_addr);
        memset(&(node_addr), 0, node_len);

        int sock = ::accept( listener.getFD(), (struct sockaddr *) &node_addr, &node_len );

        if( sock > 0 && node_addr.sin_addr.s_addr == FRONENDIP )
            {
            //Call recieve and find and send in new thread;
            //Move it into the lambda

            auto func = [this] ( int fd )
            {
                handle_query( fd );
            };
            pool.submitNoFuture( func, sock );

            }
        // If someone with a higher node id tries to connect to me write to std::cer
        // Also checks if someone who is not allowed to connect tries to connect to me
        else
        {
            std::cerr << "Connection failed wrong\n";
        }
    }
}

void Node::handle_query( int fd )
{
    //Call receive

    APESEARCH::string query = receiver( fd );

    if(query.empty())
    {
        std::cerr << "Could not read query from server\n";
        close(fd);
        return;
    }

    //Parse query
    //their API
    APESEARCH::vector<APESEARCH::string> top_ten = { "url1", "url2", "url3", "url4", "url5", "url6", "url7", "url8", "url9", "url10" };

    //Send vector
    sender( fd, top_ten);

    close(fd);
}

void Node::sender( int fd, APESEARCH::vector<APESEARCH::string> &top_ten )
{   
    for(int i = 0; i < top_ten.size( ); ++i)
    {

        ssize_t ret;
        if( i == top_ten.size() - 1 )
        {
            ret = send( fd ,top_ten[i].cstr( ), top_ten[i].size( ) + 1 , 0);
        }
        else
        {
            top_ten[i].push_back( '\n' );
            ret = send( fd, top_ten[i].cstr( ), top_ten[i].size( ) , 0);
        }

        if(ret < 0)
        {
            std::cerr << "Did not send url successfully\n";
        }
    }
}

APESEARCH::string Node::receiver( int fd )
{
    APESEARCH::string buff(BUFFERSIZE, 'a');
    
    ssize_t bytes_read;

    while(bytes_read < BUFFERSIZE)
    {
        ssize_t read = recv( fd, &buff.front( ) + bytes_read, BUFFERSIZE - bytes_read, 0 );    

        if(buff[bytes_read + read] == '\0')
            break;
    }

    return buff;
}