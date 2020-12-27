#include <boost/asio.hpp>
#include <boost/asio/buffered_stream.hpp>
#include <boost/beast.hpp>
#include <iostream>

// Test Synchronous WebSocket Client
int main(void)
{
    // store any errors
    boost::system::error_code ec {};

    // execution context for the io service
    boost::asio::io_context ioc {};

    // for this specific example, connect to a dedicated test socket over HTTP (web)
    boost::asio::ip::tcp::resolver resolver {ioc};
    auto endpoint { resolver.resolve("echo.websocket.org", "80", ec) };

    // create socket and establish connection
    boost::asio::ip::tcp::socket socket {ioc};
    std::cout << "::client::connecting to echo.websockets.org..." << std::endl;
    socket.connect(*endpoint, ec);
    std::cout << "::client::connected!" << std::endl;
    if (ec && ec != boost::asio::error::eof)
    {
        std::cerr << "::client::error -> " << ec.message() << std::endl;
        return -1;
    }
    // pass socket into beast's websocket interface to perform a HTTP handshake 
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> websocket_stream{std::move(socket)};
    // perform the actual handshake
    std::cout << "::client::handshaking..." << std::endl;
    std::cout << "::client::waiting for response..." << std::endl;
    websocket_stream.handshake("echo.websocket.org", "/", ec);
    // set stream state to read/write text
    websocket_stream.text(true);
    auto msg = std::string {"hello, echo.websocket.org!"};
    boost::asio::const_buffer msgbuffer {msg.c_str(), msg.size()};
    websocket_stream.write(msgbuffer, ec);

    if (ec && ec != boost::asio::error::eof)
    {
        std::cerr << "::client::write::error -> " << ec.message() << std::endl;
        return -2;
    }
    boost::beast::flat_buffer rmsgbuffer {};
    websocket_stream.read(rmsgbuffer, ec);

    if (ec && ec != boost::asio::error::eof)
    {
        std::cerr << "::client::read::error -> " << ec.message() << std::endl;
        return -3;
    }

    std::cout << "::client::response...\n"
              << "ECHO: "
              << boost::beast::make_printable(rmsgbuffer.data())
              << std::endl;

    return 0;
}
