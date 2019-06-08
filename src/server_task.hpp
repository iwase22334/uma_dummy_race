#ifndef SERVER_TASK_HPP
#define SERVER_TASK_HPP

#include "http_context.hpp"

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
        virtual std::unique_ptr<Task> operator()(asio::yield_context yield_context, HTTPContext& http_context) = 0;
    };

    class ReceiveHeader: public Task {
        using streambuf_ptr = std::shared_ptr<asio::streambuf>;
        streambuf_ptr receive_buff_;

    public:
        ReceiveHeader(asio::ip::tcp::socket& socket): Task(socket), receive_buff_(std::make_shared<asio::streambuf>()) {};

    public:
        std::unique_ptr<Task> operator()(asio::yield_context yield_context, HTTPContext& http_context);

    };

    class ReceiveBody: public Task {
        using streambuf_ptr = std::shared_ptr<asio::streambuf>;
        streambuf_ptr receive_buff_;

    public:
        ReceiveBody(asio::ip::tcp::socket& socket, streambuf_ptr buff): Task(socket), receive_buff_(buff) {};

    public:
        std::unique_ptr<Task> operator()(asio::yield_context yield_context, HTTPContext& http_context);
    };

    class ProcessRequest: public Task {
        asio::streambuf receive_buff_;

    public:
        ProcessRequest(asio::ip::tcp::socket& socket): Task(socket) {};

    public:
        std::unique_ptr<Task> operator()(asio::yield_context yield_context, HTTPContext& http_context);
    };

    class Send: public Task {
        std::shared_ptr<const std::string> send_data_;

    public:
        Send(asio::ip::tcp::socket& socket, std::shared_ptr<std::string> send_data) : 
            Task(socket), 
            send_data_(send_data) {}

    public:
        std::unique_ptr<Task> operator()(asio::yield_context yield_context, HTTPContext& http_context);
    };

}

#endif
