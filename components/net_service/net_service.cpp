/* FILE: net_service.cpp
 * AUTHOR: Thomas Smallridge
 * CREATED: 28/10/2014
 * CHANGELOG:
 * 28/10/2014: Created file and callback registering functionality. Implemented simple 
 * websocketpp websocket & http server to handle commands.
 */

#include "websocketpp/config/asio_no_tls.hpp"

#include "websocketpp/server.hpp"

#include <iostream>

class NetService {
public:
	typedef websocketpp::server<websocketpp::config::asio> WSServer;

public:
	//The map used to map names to callback functions, making them easy to use via message passing
	std::map<std::string, std::function<void(int)>> callbackMap;

	//NetService default constructor
	NetService(

	) : callbackMap()
	{
		registerCallback("test", [](int testValue) {
			std::cout << "Test callback value: " << testValue << std::endl;
		});
	}

	//Takes in a name and a pointer to a callback and adds that relation to the callback map.
	void registerCallback(std::string callbackName, std::function<void(int)> callback)
	{
		//Insert a new callback function to the callbackMap for handling
		callbackMap.insert(std::make_pair(callbackName, callback));
	}

private:
	//The websocket/http server (can handle requests from both)
	WSServer wss;

	void init() 
	{
		//Initialise ASIO config
		wss.init_asio();

		//Set handler for http connection requests
		wss.set_http_handler([&](websocketpp::connection_hdl hdl) {
			WSServer::connection_ptr con = wss.get_con_from_hdl(hdl);
			
			std::stringstream output;
			output << "<!doctype html><html><body>You requested "
	           << con->get_resource()
	           << "</body></html>";
    
    		// Set status to 200 rather than the default error code
    		con->set_status(websocketpp::http::status_code::ok);
    		// Set body text to the HTML created above
   			con->set_body(output.str());	
		});

		// Server listen on port 9000
		wss.listen(9000);

		// Starts the server accept loop
		wss.start_accept();

		//Start the ASIO io_service run loop
		wss.run();
	}

};
