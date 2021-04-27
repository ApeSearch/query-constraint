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

    APESEARCH::string queryLine = Index::buildQuery(APESEARCH::string(query.cbegin(),query.cend()).convertToLower());
    Index search("tests/condensed/");
    search.searchIndexChunks(queryLine.cstr());
    APESEARCH::vector<RankedEntry> top_ten = search.topTen; 

    //Send vector
    sender( fd, top_ten);

    close(fd);
}

void Node::sender( int fd, APESEARCH::vector<RankedEntry> &top_ten )
{   

    ssize_t size = 0;
    for( int i = 0; i < top_ten.size( ); ++i ) 
    {
        size += top_ten[i].url.size();
    }
    size = size + ( (sizeof( unsigned int )) * top_ten.size() ) + ( top_ten.size() * 2 );
    char buffer[size];
    char *ptr = buffer;

    for( int i = 0; i < top_ten.size( ); ++i )
    {

        for( int j = 0; j < top_ten[i].url.size( ); ++j )
            {
                *ptr = top_ten[i].url[j];
                ++ptr;
            }
            *ptr = ' ';
            ++ptr;
            snprintf(ptr, sizeof(unsigned int), "%u", (unsigned int) top_ten[i].rank);
            ptr += sizeof(unsigned);

        if( i == top_ten.size() - 1 )
        {
            // last one
            *ptr = '\0';
            ++ptr;
        }
        else
        {
            *ptr = '\n';
            ++ptr;
        }
    }
    assert( ptr == ( buffer + size ) );
    if( send( fd ,buffer , size , 0 ) < 0 )
        std::cerr << "Did not send url successfully\n";
}

APESEARCH::string Node::receiver( int fd )
{
    APESEARCH::string buff(BUFFERSIZE, 'a');
    
    ssize_t bytes_read = 0;

    while(bytes_read < BUFFERSIZE)
    {
        ssize_t read = recv( fd, &buff.front( ) + bytes_read, BUFFERSIZE - bytes_read, 0 );    

        bytes_read += read;
        
        if( !read || buff[bytes_read - 1] == '\0')
            break;
    }

    APESEARCH::string res(&buff.front(), &buff.front() + bytes_read);

    return res;
};