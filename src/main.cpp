#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>

void LogError(const std::string& prompt, const std::string& where, const boost::system::error_code& ec)
{
    std::cerr << prompt
              << (ec ? "ERROR: " : "OK")
              << (ec ? ec.message() : "")
              << (ec ? " -> " : "")
              << (ec ? where : "")
              << std::endl;
}
void OnReceive
(
    // shared data begin
    boost::beast::flat_buffer& read_buf,
    const std::string& prompt,
    // shared data end
    const boost::system::error_code& ec
)
{
    LogError(prompt, "OnReceive", ec);
    std::cout << prompt << "response -> " << boost::beast::make_printable(read_buf.data()) << std::endl;
    std::cout << prompt << "closing connection..." << std::endl;

}
void OnSend
(
    // shared data begin
    boost::beast::websocket::stream<boost::beast::tcp_stream>& ws,
    const std::string& url,
    boost::beast::flat_buffer& read_buf,
    const std::string& prompt,
    // shared data end
    const boost::system::error_code& ec
)
{
    LogError(prompt, "OnSend", ec);
    std::cout << prompt << "reading response from " << url << "..." << std::endl;
    ws.async_read(read_buf,
            [&read_buf, &prompt](const boost::system::error_code& ec, auto nBytesRead)
            {
                OnReceive(read_buf, prompt, ec);
            }
    );
}
void OnHandshake
(
    // shared data begin
    boost::beast::websocket::stream<boost::beast::tcp_stream>& ws,
    const std::string& url,
    const boost::asio::const_buffer& write_buf,
    boost::beast::flat_buffer& read_buf,
    const std::string& prompt,
    // shared data end
    const boost::system::error_code& ec
)
{
    LogError(prompt, "OnHandshake", ec);
    // tell ws to use text for messages
    ws.text(true);
    std::cout << prompt << "sending message to " << url << "..." << std::endl;
    ws.async_write(write_buf,
            [&ws, &url, &read_buf, &prompt](const boost::system::error_code& ec, auto nBytesWritten)
            {
                OnSend(ws, url, read_buf, prompt, ec);
            }
    );
}

void OnConnect
(
    // shared data begin
    boost::beast::websocket::stream<boost::beast::tcp_stream>& ws,
    const std::string& url,
    const boost::asio::const_buffer& write_buf,
    boost::beast::flat_buffer& read_buf,
    const std::string& prompt,
    // shared data end
    const boost::system::error_code& ec
)
{
    LogError(prompt, "OnConnect", ec);
    std::cout << prompt << "connected, handshaking..." << std::endl;
    ws.async_handshake(url, "/",
            [&ws, &url, &write_buf, &read_buf, &prompt](const boost::system::error_code& ec)
            {
                OnHandshake(ws, url, write_buf, read_buf, prompt, ec);
            }
    );
}

void OnResolve
(
    // shared data begin
    boost::beast::websocket::stream<boost::beast::tcp_stream>& ws,
    const std::string& url,
    const boost::asio::const_buffer& write_buf,
    boost::beast::flat_buffer& read_buf,
    const std::string& prompt,
    // shared data end
    const boost::system::error_code& ec,
    boost::asio::ip::tcp::resolver::iterator endpoint
)
{
    LogError(prompt, "OnResolve", ec);
    std::cout << prompt << "connecting..." << std::endl;
    ws.next_layer().async_connect(*endpoint,
            [&ws, &url, &write_buf, &read_buf, &prompt](const boost::system::error_code& ec)
            {
                OnConnect(ws, url, write_buf, read_buf, prompt, ec);
            }
    );
}

int main(int argc, char* argv[])
{
    // asynchronous client with event handlers
    // ---------------------------------------
    const auto prompt = std::string{"< ~ async_client ~ > $: "};
    if (argc != 3)
    {
        std::cout << prompt << "error::bad arguments" << std::endl;
        std::cout << prompt << "example usage -> <exec> <url> <port>" << std::endl;
        return -1;
    }
    const auto url = std::string{argv[1]};
    const auto port = std::string{argv[2]};
    // establish socket connection
    // ---------------------------
    std::cout << prompt << "establishing connection..." << std::endl;
    std::cout << prompt << url << ", " << port << std::endl;
    // io always needs some kind of execution context
    boost::asio::io_context ioc {};
    // initialize websocket with io service
    boost::beast::websocket::stream<boost::beast::tcp_stream> ws {ioc};
    // shared message caches
    const auto msg = std::string{"hello, websocket!"};
    boost::asio::const_buffer write_buf {msg.c_str(), msg.size()};
    boost::beast::flat_buffer read_buf {};
    // resolve our input url and port
    boost::asio::ip::tcp::resolver async_resolver {ioc};
    // entry point
    async_resolver.async_resolve(url, port, 
            [&ws, &url, &write_buf, &read_buf, &prompt](const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::iterator endpoint)
            {
                std::cout << prompt << "resolving..." << std::endl;
                OnResolve(ws, url, write_buf, read_buf, prompt, ec, endpoint);
            }
    );
    // now that everything's set up, do work
    ioc.run();

    return 0;
}
