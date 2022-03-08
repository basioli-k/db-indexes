#include <iostream>
#include <coroutine>
#include <boost/asio.hpp>

int main() {
    std::cout << "hello world\n";
    boost::asio::io_context ioc(2);
    return 0;
}