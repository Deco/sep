
#include <boost/bind.hpp>
#include <boost/asio.hpp>

void read_callback(
    bool &, boost::asio::deadline_timer &,
    const boost::system::error_code&, std::size_t
);
void wait_callback(
    boost::asio::serial_port &,
    const boost::system::error_code&
);

int main()
{
    boost::asio::io_service     io_svc;

    boost::asio::serial_port    ser_port(io_svc, "/dev/tty.usbserial-A9S3VTXD");
    ser_port.set_option(boost::asio::serial_port_base::character_size(8));
    //ser_port.set_option(boost::asio::serial_port_base::baud_rate(1000000));

    int new_baud = static_cast<int>(1000000);
    ioctl(ser_port.native_handle(), _IOW('T', 2, speed_t), &new_baud, 1);

    boost::asio::deadline_timer timeout(io_svc);

    std::vector<unsigned char> cmdData;
    cmdData.push_back(0xFF);
    cmdData.push_back(0xFF);
    cmdData.push_back(0x01); // id
    cmdData.push_back(0x04); // length
    cmdData.push_back(0x02); // instruction
    cmdData.push_back(0x2b); // param 1
    cmdData.push_back(0x01); // param 2
    unsigned char checksum;
    for(int i = 2; i < cmdData.size(); i++) {
        checksum += cmdData[i];
    }
    cmdData.push_back(~checksum);
    std::cout << "writing" << std::endl;
    boost::system::error_code ec;
    boost::asio::write(ser_port, boost::asio::buffer(&(cmdData[0]), cmdData.size()), ec);
    std::cout << "done: " << ec << std::endl;

    std::cout << "reading" << std::endl;
    char c;
    while(true) {
        boost::asio::read(ser_port, boost::asio::buffer(&c, 1));
        printf("%x\n", c);
    }
    // unsigned char  my_buffer[1];
    // bool           data_available = false;

    // while(true) {
    //     ser_port.async_read_some(
    //         boost::asio::buffer(my_buffer),
    //         boost::bind(
    //             &read_callback,
    //             boost::ref(data_available),
    //             boost::ref(timeout),
    //             boost::asio::placeholders::error,
    //             boost::asio::placeholders::bytes_transferred
    //         )
    //     );
    //     timeout.expires_from_now(boost::posix_time::milliseconds(500));
    //     timeout.async_wait(
    //         boost::bind(
    //             &wait_callback,
    //             boost::ref(ser_port),
    //             boost::asio::placeholders::error
    //         )
    //     );

    //     io_svc.run();  // will block until async callbacks are finished

    //     if (!data_available) {
    //         std::cout << "no data!" << std::endl;
    //         break;
    //     }
    // }
    std::cout << "done" << std::endl;
}

void read_callback(
    bool& data_available,
    boost::asio::deadline_timer& timeout,
    const boost::system::error_code& error,
    std::size_t bytes_transferred
) {
    if (error || !bytes_transferred) {
        // No data was read!
        data_available = false;
        return;
    }
    std::cout << "read count: " << bytes_transferred << std::endl;
    timeout.cancel();  // will cause wait_callback to fire with an error
    data_available = true;
}

void wait_callback(
    boost::asio::serial_port &ser_port,
    const boost::system::error_code& error
) {
    std::cout << "timeout?" << std::endl;
}
