#ifndef SERVER_TASK_HPP
#define SERVER_TASK_HPP

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <memory>

namespace servertask {
    namespace asio = boost::asio;

    class ServerTask {
    protected:
        asio::io_service& io_service_;
        asio::ip::tcp::socket& socket_;

    public:
        ServerTask(asio::io_service& io_service, asio::ip::tcp::socket& socket): io_service_(io_service), socket_(socket) {};
        virtual std::unique_ptr<ServerTask> operator()(asio::yield_context& yield_context) = 0;
    };

    class AcceptTask: public ServerTask {
        asio::ip::tcp::acceptor acceptor_;

    public:
        AcceptTask(asio::io_service& io_service, asio::ip::tcp::socket& socket): 
            ServerTask(io_service, socket), 
            acceptor_(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 31444))
            {};

        std::unique_ptr<ServerTask> operator()(asio::yield_context& yield_context);
    };

    class ReceiveTask: public ServerTask {
        asio::streambuf receive_buff_;

    public:
        ReceiveTask(asio::io_service& io_service, asio::ip::tcp::socket& socket): ServerTask(io_service, socket) {};

        std::unique_ptr<ServerTask> operator()(asio::yield_context& yield_context);
    };

    class SendTask: public ServerTask {
        std::shared_ptr<const std::string> send_data_;

    public:
        SendTask(asio::io_service& io_service, asio::ip::tcp::socket& socket, std::shared_ptr<std::string> send_data) : 
            ServerTask(io_service, socket), 
            send_data_(send_data) {}

        std::unique_ptr<ServerTask> operator()(asio::yield_context& yield_context);
    };

    class CloseTask: public ServerTask {
    public:
        CloseTask(asio::io_service& io_service, asio::ip::tcp::socket& socket): ServerTask(io_service, socket) {};
        std::unique_ptr<ServerTask> operator()(asio::yield_context& yield_context);
    };

}

#endif
