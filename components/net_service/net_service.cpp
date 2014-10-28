

class NetService {
public:
	typedef websocketpp::server<websocketpp::config::asio> WSServer;

public:
	std::map<std::string, std::function<void(int)> callbackMap;

	//NetService default constructor
	NetService(

	) : callbackMap()
	{
		registerCallback("test", [](int testValue) {
			std::cout << "Test callback value: " << testValue << std::endl;
		});
	}

	void registerCallback(std::string callbackName, std::function<void(int)> callback)
	{
		//Insert a new callback function to the callbackMap for handling
		callbackMap.insert(std::make_pair(callbackName, callback));
	}

private:
	websocketpp::server<websocketpp::config::asio> wss;

	void init() 
	{
		//Initialise ASIO config
		wss.init_asio();

		//Set handler for http connection requests
		wss.set_http_handler([](WSServer *svr, websocketpp::connection_hdl hdl) {
			server::connection_ptr con = svr->get_con_from_hdl(hdl);
			
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

}