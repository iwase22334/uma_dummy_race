#include "server.hpp"

#include <iostream>

int main()
{
    try {
        boost::asio::io_service io_service;
        Server server(io_service);
        io_service.run();
    }
    catch (const boost::system::system_error& e) {
        std::cerr << "exception catched" << std::endl;
        std::cerr << e.what() << std::endl;
    }
    catch (std::exception& e) {
        std::cerr << "exception: "<< e.what() << std::endl;
    }

    return 0;
}

