#include "server_task.hpp"
#include <iostream>
#include <regex>

namespace task {
    namespace asio = boost::asio;

    std::unique_ptr<Task> ReceiveTask::operator()(asio::yield_context& yield_context) {
        int content_length = 0;

        {
            std::cout << "receiving header..." << std::endl;
            boost::system::error_code error_code;

            std::size_t header_length = boost::asio::async_read_until( socket_, receive_buff_, "\r\n\r\n", yield_context[error_code]);

            if (error_code && error_code != asio::error::eof) {
                return nullptr;
            }

            std::string data(asio::buffer_cast<const char*>(receive_buff_.data()), header_length);
            receive_buff_.consume(header_length);

            std::smatch result;
            std::regex re{ R"(Content-Length: (\d+))" };
            if (!std::regex_search(data, result, re)) {
                std::cout << "no content-length" << std::endl;
            }

            else {
                try {
                    content_length = std::stoi(result[1].str());
                }

                catch (...) {
                    std::cout << "invalid content_length" << std::endl;
                    return nullptr;
                }
            }

            std::cout << data << std::endl;
        }

        {
            std::cout << "receiving body..." << std::endl;
            boost::system::error_code error_code;

            boost::asio::async_read( socket_, receive_buff_, asio::transfer_exactly(content_length - receive_buff_.size()), yield_context[error_code]);
            if (error_code) {
                return nullptr;
            }

            std::string data = asio::buffer_cast<const char*>(receive_buff_.data());
            std::cout << data << std::endl;

            receive_buff_.consume(receive_buff_.size());
        }

        return std::make_unique<SendTask>(socket_, std::make_shared<std::string>("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 2\r\n\r\nok"));
    }

    std::unique_ptr<Task> SendTask::operator()(asio::yield_context& yield_context) {
        std::cout << "sending..." << std::endl;
        boost::system::error_code error_code;

        asio::async_write(socket_, asio::buffer(*send_data_), yield_context[error_code]);
        if (error_code) {
            std::cout << "send failsed" << error_code.message() << std::endl;
            return nullptr;
        }

        return nullptr;

    }

}
