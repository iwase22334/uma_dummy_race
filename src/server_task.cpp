#include "server_task.hpp"
#include "psql_proxy.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <iostream>

namespace task {
    namespace asio = boost::asio;

    std::unique_ptr<Task> ReceiveHeader::operator()(asio::yield_context yield_context, HTTPContext& http_context) {
        std::cout << "receiving header..." << std::endl;
        std::cout << "---------------" << std::endl;
        boost::system::error_code error_code;

        std::size_t header_length = boost::asio::async_read_until( socket_, *receive_buff_, "\r\n\r\n", yield_context[error_code] );
        if ( error_code == asio::error::eof ) { 
            std::cout << "ReceiveBody" << ": " << "closed by peer" << std::endl;
            return nullptr;
        }
        else if(error_code) {
            std::cerr << "ReceiveHeader" << ": " << error_code.message() << std::endl;
            return nullptr;
        }

        std::string header(asio::buffer_cast<const char*>(receive_buff_->data()), header_length);
        receive_buff_->consume(header_length);

        // Analyze header
        if (auto contentlen = HTTPContext::get_content_length(header)) {
            http_context.content_length = contentlen.get();
        }
        else {
            std::cerr << "ReceiveHeader"  << ": " << "invalit content_length header" << std::endl;
            return nullptr;
        }

        if (auto connection = HTTPContext::get_connection(header)) {
            http_context.connection = connection.get();
        }

        if (auto transfer_encoding = HTTPContext::get_transfer_encoding(header)) {
            http_context.transfer_encoding = transfer_encoding.get();
        }

        std::cout << header << std::endl;
        std::cout << "---------------" << std::endl;
        std::cout << http_context.content_length << std::endl;
        std::cout << "---------------" << std::endl;

        return std::make_unique<ReceiveBody>(socket_, receive_buff_);
    }

    std::unique_ptr<Task> ReceiveBody::operator()(asio::yield_context yield_context, HTTPContext& http_context) {
        if (http_context.content_length < receive_buff_->size()) {
            return std::make_unique<Send>(
                    socket_, 
                    Send::generate_400_bad_request("Error")
                    );
        }

        std::cout << "receiving body..." << std::endl;
        std::cout << "---------------" << std::endl;
        std::cout << http_context.content_length - receive_buff_->size() << std::endl;
        std::cout << "---------------" << std::endl;

        {
            boost::system::error_code error_code;
            boost::asio::async_read( 
                    socket_, 
                    *receive_buff_, 
                    asio::transfer_exactly(http_context.content_length - receive_buff_->size()), 
                    yield_context[error_code]);

            if ( error_code == asio::error::eof ) { 
                std::cout << "ReceiveBody" << ": " << "closed by peer" << std::endl;
                return nullptr;
            }
            else if (error_code) {
                std::cerr << "ReceiveBody" << ": " << error_code.message() << std::endl;
                return nullptr;
            }
        }

        std::shared_ptr<const std::string> body = std::make_shared<std::string>(asio::buffer_cast<const char*>(receive_buff_->data()));
        receive_buff_->consume(receive_buff_->size());

        std::cout << body << std::endl;
        std::cout << "---------------" << std::endl;

        return std::make_unique<Application>(
                socket_, 
                body);
    }

    std::unique_ptr<Task> Application::operator()(asio::yield_context yield_context, HTTPContext& http_context) {
        (void)yield_context;
        (void)http_context;

        boost::property_tree::ptree id;
        std::istringstream is(*query_);

        try {
            boost::property_tree::read_json(is, id);
        }

        catch (const boost::property_tree::json_parser_error& e)
        {
            std::cerr << "Application" << ": " << e.what() << std::endl;
        }

        PSQLProxy ps;
        auto pt = ps.operator()<QueryHRTansyou>(id);

        if (!pt) {
            return std::make_unique<Send>(
                    socket_, 
                    Send::generate_400_bad_request(R"({ "error": "Invalid request format" })"));
        }

        std::ostringstream os;
        boost::property_tree::write_json(os, *pt);

        return std::make_unique<Send>(
                socket_, 
                Send::generate_200_ok(os.str()));
    }

    std::shared_ptr<const std::string> Send::generate_200_ok(const std::string& body){
        assert( body.size() > 0);

        std::stringstream message;

        message << "HTTP/1.1 200 OK\r\n";
        message << "Content-Type: text/html\r\n";
        message << "Content-Length: " << body.size() << "\r\n\r\n";
        message << body;

        return std::make_shared<std::string>(message.str());
    };

    std::shared_ptr<const std::string> Send::generate_400_bad_request(const std::string& body){
        assert( body.size() > 0);

        std::stringstream message;

        message << "HTTP/1.1 400 Bad Request\r\n";
        message << "Content-Type: text/html\r\n";
        message << "Content-Length: " << body.size() << "\r\n\r\n";
        message << body;

        return std::make_shared<std::string>(message.str());
    };

    std::unique_ptr<Task> Send::operator()(asio::yield_context yield_context, HTTPContext& http_context) {
        std::cout << "sending..." << std::endl;
        boost::system::error_code error_code;

        asio::async_write(socket_, asio::buffer(*send_data_), yield_context[error_code]);
        if (error_code) {
            std::cerr << "Send" << ": " << error_code.message() << std::endl;
            return nullptr;
        }

        if (http_context.connection == HTTPContext::Connection::close) {
            return nullptr;
        }

        return std::make_unique<ReceiveHeader>(socket_);
    }

}
