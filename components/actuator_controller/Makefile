
SRCDIR=.
OBJDIR=build
BINDIR=build
CCFLAGS=-I$(SRCDIR)/../core -I$(SRCDIR)/../util -I$(SRCDIR)/../serial_port -ggdb


$(BINDIR)/program: $(OBJDIR)/actuator_comm_ax12.o $(OBJDIR)/actuator_controller.o
	g++ $(CCFLAGS) -std=c++11 $(OBJDIR)/actuator_comm_ax12.o $(OBJDIR)/actuator_controller.o -o $(BINDIR)/program 

$(OBJDIR)/actuator_comm_ax12.o:
	g++ $(CCFLAGS) -std=c++11 -c $(SRCDIR)/actuator_comm_ax12.cpp -o $(OBJDIR)/actuator_comm_ax12.o
	
$(OBJDIR)/actuator_controller.o:
	g++ $(CCFLAGS) -std=c++11 -c $(SRCDIR)/actuator_controller.cpp -o $(OBJDIR)/actuator_controller.o



