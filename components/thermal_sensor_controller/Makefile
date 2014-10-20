run: program
	./program

program: sensor_controller.o application_core.o serialconn.o main.o
	g++ -std=c++11 -o program application_core.o main.o sensor_controller.o serialconn.o -lboost_system -lopencv_core

main.o: main.cpp sensor_controller.h application_core.h
	g++ -c -std=c++11 main.cpp sensor_controller.h application_core.h

sensor_controller.o: sensor_controller.cpp sensor_controller.h serialconn.h application_core.h 
	g++ -c -std=c++11 sensor_controller.cpp sensor_controller.h serialconn.h application_core.h 

application_core.o: application_core.cpp application_core.h
	g++ -c -std=c++11 application_core.cpp application_core.h

serialconn.o: serialconn.cpp serialconn.h
	g++ -c -std=c++11 serialconn.cpp serialconn.h



#not added to main project yet
serial: serial_port_bsd.cpp serial_port.h
	g++ -c -std=c++11 serial_port_bsd.cpp serial_port.h



clean:
	rm -f *.o *.gch program