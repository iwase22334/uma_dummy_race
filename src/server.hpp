#include "server_task.hpp"
#include "http_context.hpp"

#include <iostream>
#include <memory>
#include <functional>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/regex.hpp>
#include <boost/asio/spawn.hpp>

namespace asio = boost::asio;

class Connection : public std::enable_shared_from_this<Connection> {
    asio::ip::tcp::socket socket_;
    std::unique_ptr<task::Task> task_ptr_;

    HTTPContext http_context;

public:
    using connection_ptr = std::shared_ptr<Connection>;

public:
    static connection_ptr create(asio::io_service& io_service) {
        return connection_ptr(new Connection(io_service));
    }

public:
    Connection(asio::io_service& io_service)
        : socket_(io_service),
          task_ptr_(std::make_unique<task::ReceiveHeader>(socket_)) { }

    ~Connection() {
        socket_.shutdown(asio::ip::tcp::socket::shutdown_both);
        socket_.close();
    }

    asio::ip::tcp::socket& socket() { return socket_; } ;

    void work(asio::yield_context yield_context) {
        std::cout << "Start task" << std::endl;
        while (task_ptr_) {
            try {
                task_ptr_ = task_ptr_->operator()(yield_context, http_context);
            }
            catch (const boost::system::system_error& e) {
                std::cerr << "exception catched" << std::endl;
                std::cerr << e.what() << std::endl;
                return;
            }
            catch (...) {
                std::cerr << "exception catched" << std::endl;
                return;
            }
        }
        std::cout << "All task finished" << std::endl;
    }

};

class Server {
    asio::ip::tcp::acceptor acceptor_;

public:
    Server(asio::io_service& io_service) 
        : acceptor_(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 8080)) 
    {
        start_accept();
    };

private:
    void start_accept() {
        std::cout << __func__ << ": " << "start accepting..." << std::endl;
        auto connection = Connection::create(acceptor_.get_io_service());
        acceptor_.async_accept(connection->socket(), 
            boost::bind(&Server::on_accept, this, connection, asio::placeholders::error));
    }

    void on_accept(Connection::connection_ptr connection, const boost::system::error_code& error) {
        std::cout << __func__ << ": " << "accepted ..." << std::endl;
        if (!error) {
            boost::asio::spawn(acceptor_.get_io_service(), boost::bind(&Connection::work, connection, boost::placeholders::_1));
            start_accept();
        }

        else {
            std::cout << error.message() << std::endl;
        }
    }
};
