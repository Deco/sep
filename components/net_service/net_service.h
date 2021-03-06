/* FILE: net_service.h
 * AUTHOR: Thomas Smallridge
 * CREATED: 28/10/2014
 * CHANGELOG:
 * 28/10/2014: Created file and callback registering functionality. Implemented simple 
 * websocketpp websocket & http server to handle commands.
 */

#include <memory>
#include <set>
#include <boost/smart_ptr/owner_less.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include "application_core.h"
#include <iostream>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

class NetService {
public:
    typedef websocketpp::server<websocketpp::config::asio> WSServer;
    typedef std::function<void(const rapidjson::Document&)> MessageCallbackFunc;

public:
    //The map used to map names to callback functions, making them easy to use via message passing
    std::map<std::string, MessageCallbackFunc> callbackMap;

    //NetService default constructor
    NetService(
        const std::shared_ptr<ApplicationCore> &coreIn
    );

    //Takes in a name and a pointer to a callback and adds that relation to the callback map.
    void registerCallback(std::string callbackName, MessageCallbackFunc callback);

public:
    std::shared_ptr<ApplicationCore> core;
    WSServer wss;
    std::set<websocketpp::connection_hdl, boost::owner_less<websocketpp::connection_hdl>> connectionList;

    void init(int port);

    void handleWSOpen(websocketpp::connection_hdl hdl);
    void handleWSClose(websocketpp::connection_hdl hdl);
    void handleWSMessage(
        websocketpp::connection_hdl hdl,
        WSServer::message_ptr msg
    );
    void sendWSDoc(rapidjson::Document &doc);

    void handleHTTPConn(
        websocketpp::connection_hdl hdl
    );

};
