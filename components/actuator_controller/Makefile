SRCDIR=.
OBJDIR=.
BINDIR=.
CCFLAGS=-I$(SRCDIR)/../core -I$(SRCDIR)/../util -I$(SRCDIR)/../serial_port -ggdb


$(BINDIR)/program: $(OBJDIR)/actuator_comm_ax12.o $(OBJDIR)/actuator_controller.o $(OBJDIR)/serial_port_bsd.o
	g++ -std=c++11 $(CCFLAGS) -std=c++11 $(OBJDIR)/actuator_comm_ax12.o $(OBJDIR)/actuator_controller.o -o $(BINDIR)/program $(OBJDIR)/serial_port_bsd.o

$(OBJDIR)/actuator_comm_ax12.o:	$(SRCDIR)/actuator_comm_ax12.cpp
	g++ -std=c++11 $(CCFLAGS) -std=c++11 -c $(SRCDIR)/actuator_comm_ax12.cpp -o $(OBJDIR)/actuator_comm_ax12.o 
	
$(OBJDIR)/actuator_controller.o: $(SRCDIR)/actuator_controller.cpp
	g++ -std=c++11 $(CCFLAGS) -std=c++11 -c $(SRCDIR)/actuator_controller.cpp -o $(OBJDIR)/actuator_controller.o

$(OBJDIR)/serial_port_bsd.o: $(SRCDIR)/../serial_port/serial_port_bsd.cpp
	g++ -std=c++11 $(CCFLAGS) -std=c++11 -c $(SRCDIR)/../serial_port/serial_port_bsd.cpp -o $(OBJDIR)/serial_port_bsd.o