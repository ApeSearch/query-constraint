#include "../include/Node.h"
#include <iostream>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXTHREADS 128
#define BLOOMFILTER
#define BUFFERSIZE 8192
#define PORT 6666
#define FRONENDIP inet_addr("35.196.78.129")

using APESEARCH::unique_ptr;


Node::Node() : pool( MAXTHREADS, MAXTHREADS ), listener(PORT){}

Node::~Node(){}

struct Func
    {
    Func( APESEARCH::unique_ptr<Socket>&& _socket_ptr, Node *_node ) : socket_ptr( std::move( _socket_ptr ) ), node( _node ) { }
    void operator()()
       {
        node->handle_query( socket_ptr );
       } // end operator()()
    
    Func( Func&& other ) : socket_ptr( std::move( other.socket_ptr ) ), node( other.node ) { }

   APESEARCH::unique_ptr<Socket> socket_ptr;
   Node *node;
   };

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

        try
        {

            unique_ptr<Socket> sock_ptr = listener.accept((struct sockaddr *) &node_addr, &node_len);

            if( node_addr.sin_addr.s_addr == FRONENDIP )
                {
                //Call recieve and find and send in new thread;
                //Move it into the lambda
                pool.submitNoFuture( std::move( Func( std::move( sock_ptr ), this ) ) );
                }
            // If someone with a higher node id tries to connect to me write to std::cer
            // Also checks if someone who is not allowed to connect tries to connect to me
            else
            {
                std::cerr << "Connection to unknown ip cancelled, ip \n";
            }
        }
        catch(...)
        {
            std::cerr << "Failed accepting call on server socket\n";
        }
    }

}

void Node::handle_query(unique_ptr<Socket> &sock_ptr)
{
    //Call receive
    try
    {
    APESEARCH::string query = receiver(sock_ptr);

    }
    catch(...)
    {
        std::cerr << "Could not read query from server\n";
        return;
    }

    //Parse query
    //their API
    APESEARCH::vector<APESEARCH::string> top_ten = { "url1", "url2", "url3", "url4", "url5", "url6", "url7", "url8", "url9", "url10" };

    try
    {
        //Send vector
        sender(sock_ptr, top_ten);
    }
    catch(...)
    {
        std::cerr << "Could not send top ten to server\n";
        return;
    }
}

void Node::sender( unique_ptr<Socket> &sock_ptr, APESEARCH::vector<APESEARCH::string> &top_ten )
{   
    for(int i = 0; i < top_ten.size( ); ++i)
    {

        if( i == top_ten.size() - 1 )
            sock_ptr->send( top_ten[i].cstr( ), top_ten[i].size( ) + 1 );
        else
        {
            top_ten[i].push_back( '\n' );
            sock_ptr->send( top_ten[i].cstr( ), top_ten[i].size( ) );
        }

    }
}

APESEARCH::string Node::receiver( unique_ptr<Socket> &sock_ptr )
{
    APESEARCH::string buff(BUFFERSIZE, 'a');
    
    ssize_t bytes_read;

    while(bytes_read < BUFFERSIZE)
    {
        ssize_t read = sock_ptr->receive( &buff.front( ) + bytes_read, BUFFERSIZE - bytes_read);    

        if(buff[bytes_read + read] == '\0')
            break;
    }

    return buff;
}