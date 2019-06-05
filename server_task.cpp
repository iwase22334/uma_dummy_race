#include "server_task.hpp"
#include <iostream>
#include <regex>

namespace servertask {
    namespace asio = boost::asio;

    std::unique_ptr<ServerTask> AcceptTask::operator()(asio::yield_context& yield_context) {
        std::cout << "accepting..." << std::endl;

        boost::system::error_code error_code;

        acceptor_.async_accept( socket_, yield_context[error_code] );
        if (error_code) {
            std::cout << "connect failed : " << error_code.message() << std::endl;
            return std::make_unique<AcceptTask>(io_service_, socket_);
        }

        std::cout << "connected" << std::endl;
        return std::make_unique<ReceiveTask>(io_service_, socket_);
    }

    std::unique_ptr<ServerTask> ReceiveTask::operator()(asio::yield_context& yield_context) {
        int content_length = 0;

        {
            std::cout << "recv_header" << std::endl;
            boost::system::error_code error_code;

            std::size_t header_length = boost::asio::async_read_until( socket_, receive_buff_, "\r\n\r\n", yield_context[error_code]);

            if (error_code && error_code != asio::error::eof) {
                return std::make_unique<CloseTask>(io_service_, socket_);
            }

            std::string data(asio::buffer_cast<const char*>(receive_buff_.data()), header_length);
            receive_buff_.consume(header_length);

            std::smatch result;
            std::regex re{R"(Content-Length: (\d+))"};
            if (!std::regex_search(data, result, re)) {
                std::cout << "no content-length" << std::endl;
            }
            else {
                try {
                    content_length = std::stoi(result[1].str());
                }

                catch (...) {
                    std::cout << "invalid content_length" << std::endl;
                    return std::make_unique<CloseTask>(io_service_, socket_);
                }
            }

            std::cout << data << std::endl;
        }

        {
            std::cout << "recv_body" << std::endl;
            boost::system::error_code error_code;
            boost::asio::async_read( socket_, receive_buff_, asio::transfer_exactly(content_length - receive_buff_.size()), yield_context[error_code]);
            if (error_code) {
                return std::make_unique<CloseTask>(io_service_, socket_);
            }

            std::string data = asio::buffer_cast<const char*>(receive_buff_.data());
            std::cout << data << std::endl;

            receive_buff_.consume(receive_buff_.size());
        }

        return std::make_unique<SendTask>(io_service_, socket_, std::make_shared<std::string>("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 2\r\n\r\nok"));
    }

    std::unique_ptr<ServerTask> SendTask::operator()(asio::yield_context& yield_context) {
        std::cout << "sending..." << std::endl;
        boost::system::error_code error_code;

        asio::async_write(socket_, asio::buffer(*send_data_), yield_context[error_code]);
        if (error_code) {
            std::cout << "send failsed" << error_code.message() << std::endl;
            return std::make_unique<AcceptTask>(io_service_, socket_);
        }

        return std::make_unique<CloseTask>(io_service_, socket_);

    }

    std::unique_ptr<ServerTask> CloseTask::operator()(asio::yield_context& yield_context) {
        std::cout << "closing..." << std::endl;

        (void)yield_context;
        boost::system::error_code error_code;

        socket_.shutdown(asio::ip::tcp::socket::shutdown_both, error_code);
        if (error_code) {
            std::cout << "shutdown failsed" << error_code.message() << std::endl;
        }

        socket_.close();

        return std::make_unique<AcceptTask>(io_service_, socket_);
    }

}
