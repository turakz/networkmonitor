#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H
#include <boost/asio.hpp>
#include <boost/beast.hpp>
namespace network_monitor {
    class websocket_client {
    public:
        websocket_client(
            boost::asio::io_context& ioc,
            const std::string& url,
            const std::string& port
        );
        ~websocket_client();
        void connect();
        void send_request(const std::string& message, boost::system::error_code& ec);
        void listen_for_response(boost::system::error_code& ec);
        void disconnect();
    private:
        std::string prompt_{"<~client~>: "};
        std::string url_;
        std::string port_;
        boost::asio::ip::tcp::resolver resolver_;
        boost::beast::websocket::stream<boost::beast::tcp_stream> websocket_;
        boost::beast::flat_buffer rBuf_;
        void on_resolve(
            boost::system::error_code& ec,
            boost::asio::ip::tcp::resolver::iterator endpoint
        );
        void on_connect(boost::system::error_code& ec);
        void on_handshake(boost::system::error_code& ec);
        void on_send(boost::system::error_code& ec);
        void on_read(boost::system::error_code& ec, size_t nBytes);
        void on_response(boost::system::error_code& ec, const std::string& message);
        void on_disconnect(boost::system::error_code& ec);
    };
}
#endif
