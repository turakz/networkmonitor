#include "websocket-client.h"
#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <iostream>
#include <iomanip>
#include <string>

// static functions
static void Log(const std::string& what, boost::system::error_code& ec)
{
    std::cerr << '[' << std::setw(20) << "::client::status::"
              << (ec ? "error::" : "OK...")
              << (ec ? ec.message() : "")
              << "::" << what << ']'
              << std::endl;
}
// ctors
network_monitor::websocket_client::websocket_client(boost::asio::io_context& ioc,
                                                    const std::string& url,
                                                    const std::string& port)
    : resolver_(boost::asio::make_strand(ioc)), websocket_(boost::asio::make_strand(ioc)), url_(url), port_(port)
{
    std::cout << prompt_ << "initializing..." << std::endl;
}
network_monitor::websocket_client::~websocket_client() = default;
// api call
void network_monitor::websocket_client::connect()
{
    std::cout << prompt_ << "resolving " << url_ << ":" << port_ << "..." << std::endl;
    resolver_.async_resolve(url_, port_,
        [this](auto ec, auto endpoint)
        {
            on_resolve(ec, endpoint);
        }    
    );
}
void network_monitor::websocket_client::send_request(const std::string& message, boost::system::error_code& ec)
{
    if (ec)
    {
        Log("network_monitor::websocket_client::send_request(message, error_code)", ec);
        return;
    }
    websocket_.async_write(boost::asio::buffer(std::move(message)),
        [this](auto ec, auto)
        {
            on_send(ec);
        }
    );
}
void network_monitor::websocket_client::listen_for_response(boost::system::error_code& ec)
{
    if (ec == boost::asio::error::operation_aborted)
    {
        return;
    }
    std::cout << prompt_ << "listening..." << std::endl;
    websocket_.async_read(rBuf_,
        [this](auto ec, auto nBytes)
        {
            on_read(ec, nBytes);
            listen_for_response(ec);
        }
    );
}
void network_monitor::websocket_client::disconnect()
{
    websocket_.async_close(boost::beast::websocket::close_code::none,
        [this](auto ec)
        {
            on_disconnect(ec);
        }
    );
}
// event handlers
void network_monitor::websocket_client::on_resolve(boost::system::error_code& ec,
                                                   boost::asio::ip::tcp::resolver::iterator endpoint)
{
    if (ec)
    {
        Log("network_monitor::websocket_client::on_resolve(error_code, endpoint)", ec);
        return;
    }
    std::cout << prompt_ << "connecting..." << std::endl; 

    // set a timeout for the tcp socket
    websocket_.next_layer().async_connect(*endpoint,
        [this](auto ec)
        {
            on_connect(ec);
        }
    );
}
void network_monitor::websocket_client::on_connect(boost::system::error_code& ec)
{
    if (ec)
    {
        Log("network_monitor::websocket_client::on_connect(error_code)", ec);
        // send error response
        send_request("hello server!", ec);
        return;
    }
    std::cout << prompt_ << "connected..." << std::endl;

    // reset timeout to something beast suggests now that we have a tcp connection
    websocket_.next_layer().expires_never();
    websocket_.set_option(boost::beast::websocket::stream_base::timeout::suggested(
        boost::beast::role_type::client
    ));
    std::cout << prompt_ << "handshaking over secure tcp stream..." << std::endl;
    websocket_.async_handshake(url_, "/",
        [this](auto ec)
        {
            on_handshake(ec);
        }
    );
}
void network_monitor::websocket_client::on_handshake(boost::system::error_code& ec)
{
    if (ec)
    {
        Log("network_monitor::websocket_client::on_handshake(error_code)", ec);
        // send error response
        send_request("hello server!", ec);
        return;
    }
    // tell websocket to exchange messages in a text stream
    websocket_.text(true);
    // set up an asynchronous listening loop to receive messages
    listen_for_response(ec);
    // dispatch other callbacks
    send_request("hello server!", ec);
}
void network_monitor::websocket_client::on_send(boost::system::error_code& ec)
{
    if (ec)
    {
        Log("network_monitor::websocket_client::on_send", ec);
        return;
    }
}
void network_monitor::websocket_client::on_read(boost::system::error_code& ec, size_t nBytes)
{
    // ignore messages that failed on read
    if (ec) return;
    // parse response and forward it to callback
    std::string response {boost::beast::buffers_to_string(rBuf_.data())};
    rBuf_.consume(nBytes);
    on_response(ec, response);
}
void network_monitor::websocket_client::on_response(boost::system::error_code& ec, const std::string& response)
{
    if (ec)
    {
        Log("network_monitor::websocket_client::on_response(error_code, response)", ec);
        return;
    }
    std::cout << prompt_ << "response received -> " << response << std::endl;
    std::cout << prompt_ << "disconnecting..." << std::endl;
    disconnect();
}
void network_monitor::websocket_client::on_disconnect(boost::system::error_code& ec)
{
    //do something after disconnect...
    std::cout << prompt_ << "disconnected..." << std::endl;
}
