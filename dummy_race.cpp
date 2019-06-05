#include "server_task.hpp"

#include <memory>
#include <functional>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/regex.hpp>
#include <boost/asio/spawn.hpp>

namespace asio = boost::asio;

class Server {
    asio::io_service& io_service_;
    asio::ip::tcp::socket socket_;
    std::unique_ptr<servertask::ServerTask> server_task_ptr_;

public:
    Server(asio::io_service& io_service)
        : io_service_(io_service),
          socket_(io_service),
          server_task_ptr_(new servertask::AcceptTask(io_service_, socket_)) {}

    void work(asio::yield_context yield_context) {
        while (true) {
            //server_task_ptr_.reset(server_task_ptr_->operator()(yield_context));
            server_task_ptr_ = server_task_ptr_->operator()(yield_context);
        }
    }

};

int main()
{
    asio::io_service io_service;

    Server server(io_service);

    asio::spawn(io_service, std::bind(&Server::work, &server, std::placeholders::_1));
    io_service.run();

    return 0;
}

