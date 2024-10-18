#include <iostream>
#include <cstring>
#include <boost/asio.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

using namespace std;
using namespace boost;
using boost::asio::ip::tcp;

int main(){
    system::error_code ec;
    asio::io_context io;

    std::cout << "Boost version: " << BOOST_VERSION << std::endl;

    tcp::endpoint endpoint(asio::ip::make_address("93.184.216.34",ec),80);

    tcp::socket socket(io);

    socket.connect(endpoint,ec);

    if(!ec) cout << "connected" << endl;
    else cout << ec.message() << endl;

    return 0;
}

