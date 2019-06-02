#include <iostream>
#include <functional>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/regex.hpp>
#include <boost/asio/spawn.hpp>


namespace asio = boost::asio;

class Session{
    asio::coroutine coroutine_;

public:
    void operator()(asio::streambuf& buf) {
        
    }

private:
    void receive_header(asio::streambuf& buf) {
        
    }
}

class Server {
    asio::io_service& io_service_;
    asio::ip::tcp::acceptor acceptor_;
    asio::ip::tcp::socket socket_;
    asio::streambuf receive_buff_;

public:
    Server(asio::io_service& io_service)
        : io_service_(io_service),
          acceptor_(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 31444)),
          socket_(io_service) {}

    void work(asio::yield_context yield_context) {
        accept(yield_context);
        receive(yield_context);
        send(yield_context);
        close();
    }

private:
    void accept(asio::yield_context& yield_context) {
        boost::system::error_code error_code;

        acceptor_.async_accept( socket_, yield_context[error_code] );
        if (error_code) {
            std::cout << "connect failec : " << ec.message() << std::endl;
        }

        else {
            std::cout << "connected" << error_code.message();
        }
    }

    void receive(asio::yield_context& yield_context) {
        boost::system::error_code error_code;
        boost::asio::async_read_until( socket_, receive_buff_, "\r\n\r\n", yield_context[error_code]);


        boost::asio::async_read( socket_, receive_buff_, asio::transfer_all(), yield_context[error_code]);

    }

    void on_receive_header(const boost::system::error_code& error, size_t bytes_transferred) {
        std::cout << "recv_header" << std::endl;
        if (!error && error != asio::error::eof) {
            const char* data = asio::buffer_cast<const char*>(receive_buff_.data());
            std::cout << data << std::endl;

            receive_buff_.consume(receive_buff_.size());
        }
    }

    void on_receive_body(const boost::system::error_code& error, size_t bytes_transferred) {
        std::cout << "recv_body" << std::endl;
        if (error && error != asio::error::eof) {
            std::cout << "recv_body" << std::endl;
        }

        const char* data = asio::buffer_cast<const char*>(receive_buff_.data());
        std::cout << data << std::endl;

        receive_buff_.consume(receive_buff_.size());

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

