ifneq ($(CROSS_COMPILE),)
CROSS-COMPILE:=$(CROSS_COMPILE)
endif

CC:=$(CROSS-COMPILE)gcc
LD:=$(CROSS-COMPILE)ld


Aslibrary: clean
	$(CC) -c ecm_demo_atctl.c ecm_demo_msg.c ecm_demo_ttydev.c ecm_demo_aux.c ecm_demo_autocfg.c
	ar -crvs libecmcall.a ecm_demo_atctl.o ecm_demo_msg.o ecm_demo_ttydev.o ecm_demo_aux.o ecm_demo_autocfg.o
	$(CC) ecm_demo_main.c -g -v -static -L ./ libecmcall.a -o ECM_DEMO -lpthread -ldl -lrt
	$(CC) ecm_demo_mainauto.c -g -v -static -L ./ libecmcall.a -o ECM_DEMO_AUTO -lpthread -ldl -lrt
	rm -rf *.o

Adlibrary: clean
	$(CC) -fPIC -shared ecm_demo_atctl.c ecm_demo_msg.c ecm_demo_ttydev.c ecm_demo_aux.c ecm_demo_autocfg.c -o libecmcall.so -lpthread -ldl -lrt
	$(CC) -o ECM_DEMO_AUTO ecm_demo_mainauto.c -L. -lpthread -ldl -lrt -lecmcall
	$(CC) -o ECM_DEMO       ecm_demo_main.c -L. -lpthread -ldl -lrt -lecmcall

clean:
	rm -rf ECM_DEMO ECM_DEMO_AUTO *.so *.a *.o

