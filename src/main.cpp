#include <iostream>
#include <boost/asio.hpp>

int main(void)
{
    boost::system::error_code ec {};
    if (ec)
    {
        std::cerr << "Error: " << ec.message() << std::endl;
        return -1;
    }
    else
    {
        std::cout << "OK" << std::endl;
    }
    return 0;
}
