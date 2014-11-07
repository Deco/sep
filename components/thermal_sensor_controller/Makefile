
GCC = g++ -std=c++11
LIBS = -lboost_system -lopencv_core -lboost_thread
OBJECTS = main.o application_core.o sensor_controller.o serial_port_bsd.o params.o serialconn.o
UTIL_HEADERS = common.h events.h threads.h
#run: program
#	./program

program: ${OBJECTS}
	${GCC} -o program ${OBJECTS} ${LIBS} ${UTIL_HEADERS}

main.o: main.cpp sensor_controller.h application_core.h serial_port.h events.h threads.h
	${GCC} -c main.cpp sensor_controller.h application_core.h serial_port.h events.h threads.h

sensor_controller.o: sensor_controller.cpp sensor_controller.h serialconn.h application_core.h events.h threads.h
	${GCC} -c sensor_controller.cpp sensor_controller.h serialconn.h application_core.h events.h threads.h

application_core.o: application_core.cpp application_core.h
	${GCC} -c application_core.cpp application_core.h

params.o: params.cpp params.h threads.h
	${GCC} -c params.cpp params.h threads.h


#serialconn.o: serialconn.cpp serialconn.h
#	g++ -c -std=c++11 serialconn.cpp serialconn.h

serial_port_bsd.o: serial_port_bsd.cpp serial_port.h ${UTIL_HEADERS}
	${GCC} -c serial_port_bsd.cpp serial_port.h ${UTIL_HEADERS}


clean:
	rm -f *.o *.gch program