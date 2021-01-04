#include "websocket-client.h"

#include <boost/asio.hpp>

#include <iostream>
#include <string>

int main()
{
    // Connection targets
    const std::string url {"echo.websocket.org"};
    const std::string port {"80"};

    // Always start with an I/O context object.
    boost::asio::io_context ioc {};

    network_monitor::websocket_client client(ioc, url, port);
    client.connect();
    ioc.run();
    return 0;
}
