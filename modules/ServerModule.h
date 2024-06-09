#include <ctime>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>


using boost::asio::ip::tcp;


//---------------------- Class that represent each connection ---------------------------//
class tcp_connection
        : public std::enable_shared_from_this<tcp_connection>
{
public:
    typedef std::shared_ptr<tcp_connection> pointer;
    typedef std::function<void()> CloseCallback;

    static pointer create(boost::asio::io_context& io_context);
    tcp::socket& socket();
    static std::string make_daytime_string();
    void start();
    std::string get_ip();
    int get_port();
    void set_close_callback(const CloseCallback& callback);
    std::string get_latest_data();



private:
    explicit tcp_connection(boost::asio::io_context& io_context) : socket_(io_context){}
    void handle_write(const boost::system::error_code& /*error*/, size_t /*bytes_transferred*/);
    void start_read();
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);


// Private Vars
private:
    CloseCallback close_callback_;
    tcp::socket socket_;
    std::string message_;
    char data_[128]{};


};


//-------------------------- Server Class --------------------------//
class tcp_server
{
public:
    explicit tcp_server(boost::asio::io_context& io_context);

    [[nodiscard]] int get_connections_count() const;
    [[nodiscard]] std::vector<tcp_connection::pointer> get_connections() const {
        return connections_;
    }

private:
    void start_accept();
    void handle_accept(const tcp_connection::pointer& new_connection,
                       const boost::system::error_code& error);

    // Private Vars
private:
    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
    int connection_count_ = 0;
    std::vector<tcp_connection::pointer> connections_;
};