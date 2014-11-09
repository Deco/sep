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
	registerCallback("test", [](int testValue) {
		std::cout << "Test callback value: " << testValue << std::endl;
	});
}

//Takes in a name and a pointer to a callback and adds that relation to the callback map.
void NetService::registerCallback(
	std::string callbackName,
	std::function<void(int)> callback
)
{
	//Insert a new callback function to the callbackMap for handling
	callbackMap.insert(std::make_pair(callbackName, callback));
}

void NetService::init()
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
	wss.listen(9008);

	// Starts the server accept loop
	wss.start_accept();

	//Start the ASIO io_service run loop
	//wss.run();
}

void NetService::handleWSMessage(
	websocketpp::connection_hdl hdl,
	WSServer::message_ptr msg
)
{
	std::cout << "on_message called with hdl: " << hdl.lock().get()
              << " and message: " << msg->get_payload()
              << std::endl;
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



