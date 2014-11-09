/* FILE: net_service.cpp
 * AUTHOR: Thomas Smallridge
 * CREATED: 28/10/2014
 * CHANGELOG:
 * 28/10/2014: Created file and callback registering functionality. Implemented simple 
 * websocketpp websocket & http server to handle commands.
 */

#include <string>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "net_service.h"

namespace fs = boost::filesystem;

NetService::NetService(
	const std::shared_ptr<ApplicationCore> &coreIn
) : core(coreIn)
  , callbackMap()
{
	// 
}

//Takes in a name and a pointer to a callback and adds that relation to the callback map.
void NetService::registerCallback(
	std::string callbackName,
	MessageCallbackFunc callback
)
{
	//Insert a new callback function to the callbackMap for handling
	callbackMap.insert(std::make_pair(callbackName, callback));
}

void NetService::init(int port)
{
	//Initialise ASIO config
	wss.init_asio(core->getIOService().get());

	//Set handler for http connection requests
	wss.set_http_handler(
		std::bind(&NetService::handleHTTPConn, this,
			std::placeholders::_1
		)
	);

	wss.set_message_handler(
		std::bind(&NetService::handleWSMessage, this,
			std::placeholders::_1, std::placeholders::_2
		)
	);

	// Server listen on port 9000
	wss.listen(port);

	// Starts the server accept loop
	wss.start_accept();

	std::cout << "Accepting on port " << port << "!" << std::endl;

	//Start the ASIO io_service run loop
	//wss.run();
}

void NetService::handleWSMessage(
	websocketpp::connection_hdl hdl,
	WSServer::message_ptr msg
)
{
	rapidjson::Document doc;
	doc.Parse(msg->get_payload().c_str());

	if(!doc.IsObject()) {
		std::cout << "!!!! wtf: " << msg->get_payload() << std::endl;
		return;
	}

	auto callbackIt = callbackMap.find(doc["type"].GetString());
	if(callbackIt != callbackMap.end()) {
		callbackIt->second(doc);
	} else {
		std::cout << "Unhandled message! (" << doc["type"].GetString() << ")" << std::endl;
	}
}

void NetService::handleHTTPConn(
	websocketpp::connection_hdl hdl
)
{
	WSServer::connection_ptr con = wss.get_con_from_hdl(hdl);

	fs::path wwwpath("res/www");

	fs::path filepath = wwwpath / fs::path(con->get_resource());

	std::cout << "serving: " << filepath << std::endl;

	std::ifstream file;
	file.open(filepath.string());
	
	std::stringstream output;
	output << file.rdbuf();

   	file.close();

	// Set status to 200 rather than the default error code
	con->set_status(websocketpp::http::status_code::ok);
	// Set body text to the HTML created above
	con->set_body(output.str());

	//std::cout << output.str() << std::endl;
}



