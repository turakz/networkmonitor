#include <boost/asio.hpp>
#include <boost/asio/strand.hpp>
#include <iostream>
#include <iomanip>
#include <thread>

void Log(const boost::system::error_code& ec)
{
    std::cerr << "[" << std::setw(14) << std::this_thread::get_id() << "] "
              << (ec ? "Error" : "OK")
              << (ec ? ec.message() : "")
              << std::endl;
}

void OnConnect(const boost::system::error_code& ec)
{
    Log(ec);
}

int main(void)
{
    std::cerr << "[" << std::setw(14) << std::this_thread::get_id() << "] main" << std::endl;
    boost::asio::io_context ioc {};

    boost::asio::ip::tcp::socket socket {boost::asio::make_strand(ioc)};

    size_t threadCount {4};

    boost::system::error_code ec {};

    boost::asio::ip::tcp::resolver resolver {ioc};
    auto endpoint {resolver.resolve("google.com", "80", ec)};
    if (ec)
    {
        Log(ec);
        return -1;
    }
    for (size_t idx {0}; idx < threadCount; ++idx)
    {
        socket.async_connect(*endpoint, OnConnect);
    }
    auto threads = std::vector<std::thread>{};
    for (size_t idx {0}; idx < threadCount; ++idx)
    {
        threads.emplace_back([&ioc]()
            {
                ioc.run();
            }
        );
    }
    for (size_t idx {0}; idx < threadCount; ++idx)
    {
        threads[idx].join();
    }
    return 0;
}
