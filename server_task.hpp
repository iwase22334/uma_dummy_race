#ifndef SERVER_TASK_HPP
#define SERVER_TASK_HPP

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <memory>

namespace task {
    namespace asio = boost::asio;

    class Task {
    protected:
        asio::ip::tcp::socket& socket_;

    public:
        Task(asio::ip::tcp::socket& socket): socket_(socket) {};

    public:
        virtual std::unique_ptr<Task> operator()(asio::yield_context& yield_context) = 0;
    };

    class ReceiveTask: public Task {
        asio::streambuf receive_buff_;

    public:
        ReceiveTask(asio::ip::tcp::socket& socket): Task(socket) {};

    public:
        std::unique_ptr<Task> operator()(asio::yield_context& yield_context);
    };

    class SendTask: public Task {
        std::shared_ptr<const std::string> send_data_;

    public:
        SendTask(asio::ip::tcp::socket& socket, std::shared_ptr<std::string> send_data) : 
            Task(socket), 
            send_data_(send_data) {}

    public:
        std::unique_ptr<Task> operator()(asio::yield_context& yield_context);
    };

}

#endif
