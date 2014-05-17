#include <iostream>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "picojson.h"
#include "wsserver.h"

using websocketpp::lib::bind;

typedef websocketpp::server<websocketpp::config::asio> server;

sf::Mutex *drMutexPtr;
bool *drIsConnectedPtr;
double *drAlphaPtr, *drBetaPtr, *drGammaPtr;

void on_message(server* s, websocketpp::connection_hdl hdl, server::message_ptr msg) {
    //std::cout << "RX:" << msg->get_payload() << std::endl;

    const std::string &payload = msg->get_payload();
    picojson::value v;
    std::string err;
    picojson::parse(v, payload.begin(), payload.end(), &err);
    if (! err.empty()) {
        std::cerr << err << std::endl;
        return;
    }
    if (! v.is<picojson::object>()) {
        std::cerr << "JSON is not an object" << std::endl;
        return;
    }
    const picojson::value::object& obj = v.get<picojson::object>();
    for(picojson::value::object::const_iterator i = obj.begin();
        i != obj.end(); ++i
    ) {
        sf::Lock drMutexLock(*drMutexPtr);
        if(i->first == "alpha") {
            *drAlphaPtr = i->second.get<double>();
            //std::cout << "A: " << *drAlphaPtr << std::endl;
        } else if(i->first == "beta") {
            *drBetaPtr = i->second.get<double>();
        } else if(i->first == "gamma") {
            *drGammaPtr = i->second.get<double>();
        }
        *drIsConnectedPtr = true;
    }

    /*try {
        s->send(hdl, msg->get_payload(), msg->get_opcode());
    } catch (const websocketpp::lib::error_code& e) {
        std::cout << "Echo failed because: " << e
                  << "(" << e.message() << ")" << std::endl
        ;
    }*/
}

void wsdFunc(
    bool &drIsConnectedRef,
    double &drAlphaRef, double &drBetaRef, double &drGammaRef,
    sf::Mutex &drMutexRef,
    int port
) {
    drIsConnectedRef = false;
    drIsConnectedPtr = &drIsConnectedRef;

    drAlphaPtr = &drAlphaRef;
    drBetaPtr  = &drBetaRef ;
    drGammaPtr = &drGammaRef;
    drMutexPtr = &drMutexRef;

    server print_server;

    print_server.set_message_handler(bind(&on_message,&print_server,::_1,::_2));

    print_server.clear_access_channels(websocketpp::log::alevel::all);
    print_server.set_access_channels(websocketpp::log::alevel::connect);
    print_server.set_access_channels(websocketpp::log::alevel::disconnect);

    print_server.init_asio();
    print_server.listen(port);
    print_server.start_accept();


    std::cout << "Yay!" << std::endl;


    print_server.run();
}
