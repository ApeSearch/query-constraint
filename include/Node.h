#pragma once

#ifndef NODE_H_AS
#define NODE_H_AS 

#include "../libraries/AS/include/AS/vector.h"
#include "../libraries/AS/include/AS/string.h"
#include "../libraries/AS/include/AS/Socket.h"
#include "../libraries/AS/include/AS/unique_ptr.h"
#include "../Parser/HtmlParser.h"
#include "../libraries/AS/include/AS/pthread_pool.h"
#include "DynamicBuffer.h"
//#include "SetOfUrls.h"



class Node
{
   using CircBuf = APESEARCH::circular_buffer< APESEARCH::Func, APESEARCH::dynamicBuffer< APESEARCH::Func > >;
   APESEARCH::PThreadPool< APESEARCH::circular_buffer<APESEARCH::Func, APESEARCH::DEFAULT::defaultBuffer<APESEARCH::Func, 100u>> > pool;
    Socket listener;

public:
    //Try to connect to other nodes from ips
    //Start listening server
    //Check if swap files exist and how much data they have in them currently
    //Must have ips in some ordering!
    Node();
    ~Node();



    //1 dedicated thread-blocking
    void connectionHandler();

    void handle_query(APESEARCH::unique_ptr<Socket> &sock_ptr);

    //On functioncall stack
    //Try to send n times 
    //If cannot send write to local file
    void sender( APESEARCH::unique_ptr<Socket> &sock, APESEARCH::vector<APESEARCH::string> &top_ten );

    // 7 dedicated threads non-blocking
    APESEARCH::string receiver( APESEARCH::unique_ptr<Socket> &sock_ptr );
};

#endif