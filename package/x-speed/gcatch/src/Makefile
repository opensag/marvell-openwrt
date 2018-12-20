GPULSPULS=$(CXX)

all: DiagLog
DiagLog:main.o  DiagCmd.o DiagPort.o CrcTool.o inifile.o DiagCommandGenerator.o DmcFilePart.o
	$(GPULSPULS)  -o  DiagLog $(CFLAGS) main.o  DiagCmd.o DiagPort.o CrcTool.o inifile.o DiagCommandGenerator.o DmcFilePart.o -lpthread

main.o:main.cpp DiagCmd.h DiagPort.h comm_def.h inifile.h DiagCommandGenerator.h DmcFilePart.h
	$(GPULSPULS) $(CFLAGS) -c main.cpp

DiagCmd.o:DiagCmd.cpp DiagCmd.h DiagPort.h comm_def.h DiagPort.h
	$(GPULSPULS) $(CFLAGS) -c DiagCmd.cpp

DiagPort.o:DiagPort.cpp DiagPort.h comm_def.h
	$(GPULSPULS) $(CFLAGS) -c DiagPort.cpp

CrcTool.o:CrcTool.cpp CrcTool.h comm_def.h 
	$(GPULSPULS) $(CFLAGS) -c CrcTool.cpp


inifile.o: inifile.c inifile.h
	$(GPULSPULS) $(CFLAGS) -c inifile.c


DiagCommandGenerator.o: DiagCommandGenerator.cpp DiagCommandGenerator.h DmcFilePart.h
	$(GPULSPULS) $(CFLAGS) -c DiagCommandGenerator.cpp

DmcFilePart.o: DmcFilePart.cpp DmcFilePart.h
	$(GPULSPULS) $(CFLAGS) -c DmcFilePart.cpp

clean:
	rm -rf *.o
