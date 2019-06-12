#ifndef HTTP_CONTEXT_H
#define HTTP_CONTEXT_H

#include <boost/optional.hpp>
#include <boost/tokenizer.hpp>
#include <regex>
#include <list>

struct TransferEncoding{ 
    static constexpr unsigned char none     = 0x00;
    static constexpr unsigned char chunked  = 0x01;
    static constexpr unsigned char compress = 0x02;
    static constexpr unsigned char deflate  = 0x04;
    static constexpr unsigned char gzip     = 0x08;
    static constexpr unsigned char identity = 0x10;
};

struct HTTPContext {
    enum struct Connection { keepalive, close };

    using transferenc_type = unsigned char;
    using contentlen_type = unsigned long long;

    Connection       connection;
    transferenc_type transfer_encoding;
    contentlen_type  content_length;

    HTTPContext() 
        : connection(Connection::keepalive), 
          transfer_encoding(TransferEncoding::none), 
          content_length(0)
    {};

    static boost::optional<HTTPContext::Connection> get_connection(const std::string& header) {
        std::regex re{ R"(Connection: (.*))", std::regex::icase };
        std::smatch result;
        if (!std::regex_search(header, result, re)) {
            return boost::none;
        }

        if (result[1].str() == "close") {
            return HTTPContext::Connection::close;
        }

        if (result[1].str() == "keep-alive") {
            return HTTPContext::Connection::keepalive;
        }

        return boost::none;
    }

    static boost::optional<HTTPContext::transferenc_type> get_transfer_encoding(const std::string& header) {
        HTTPContext::transferenc_type tencoding = TransferEncoding::none;

        std::regex re{ R"(Transfer-Encoding: (.*))", std::regex::icase };
        std::smatch result;
        if (!std::regex_search(header, result, re)) {
            return boost::none;
        }

        auto find_enctype = [] (const std::string& token) -> HTTPContext::transferenc_type {
            std::list< std::pair<const char* , unsigned char> > conv_table
            {
                    { "chunked",  TransferEncoding::chunked    },
                    { "compress", TransferEncoding::compress   },
                    { "deflate",  TransferEncoding::deflate    },
                    { "gzip",     TransferEncoding::gzip       },
                    { "identity", TransferEncoding::identity   },
            };

            for ( const auto& pair: conv_table ) {
                if (pair.first == token) {
                    return pair.second;
                }
            }

            return TransferEncoding::none;
        };

        boost::tokenizer<> tokens(result[1].str());
        std::for_each (tokens.begin(), tokens.end(), [&](const auto& token) { 
            tencoding |= find_enctype(token);
        });

        return boost::none;
    }

    static boost::optional<HTTPContext::contentlen_type> get_content_length(const std::string& header) {
        HTTPContext::contentlen_type contentlen = 0;

        std::smatch result; std::regex re{ R"(Content-Length: (\d+))", std::regex::icase };
        if (!std::regex_search(header, result, re)) {
            return 0;
        }

        else {
            try {
                contentlen = std::stoull(result[1].str());
            }

            catch (...) {
                return boost::none;
            }
        }

        return contentlen;
    }

};

#endif
