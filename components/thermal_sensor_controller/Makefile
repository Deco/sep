
GCC = g++ -std=c++11
LIBS = -lboost_system -lopencv_core -lboost_thread
OBJECTS = main.o application_core.o sensor_controller.o serial_port_bsd.o

#run: program
#	./program

program: ${OBJECTS}
	${GCC} -o program ${OBJECTS} ${LIBS}

main.o: main.cpp sensor_controller.h application_core.h serial_port.h
	${GCC} -c main.cpp sensor_controller.h application_core.h serial_port.h

sensor_controller.o: sensor_controller.cpp sensor_controller.h serialconn.h application_core.h 
	${GCC} -c sensor_controller.cpp sensor_controller.h serialconn.h application_core.h 

application_core.o: application_core.cpp application_core.h
	${GCC} -c application_core.cpp application_core.h

#serialconn.o: serialconn.cpp serialconn.h
	#g++ -c -std=c++11 serialconn.cpp serialconn.h

serial_port_bsd.o: serial_port_bsd.cpp serial_port.h
	g++ -c -std=c++11 serial_port_bsd.cpp serial_port.h


clean:
	rm -f *.o *.gch program