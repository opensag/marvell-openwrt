HELP:

1. How to generate dynamic link library files and executable files.
   a).Enter demo code file directory.
      user@ubuntu:~/Desktop/ECM_DEMO$ 
   b).Do make
      user@ubuntu:~/Desktop/ECM_DEMO$ sudo make Adlibrary
   c).Then generate file:ECM_DEMO, ECM_DEMO_AUTO and libecmcall.so, ECM_DEMO and ECM_DEMO_AUTO is the executable file and libecmcall.so is the dynamic link library file.
      User need put the libecmcall.so under directory "/usr/lib", then call the file ECM_DEMO or ECM_DEMO_AUTO. Example:

      user@ubuntu:~/Desktop/ECM_DEMO$ sudo cp ./libecmcall.so /usr/lib/
      user@ubuntu:~/Desktop/ECM_DEMO$ sudo ./ECM_DEMO -t up
	  user@ubuntu:~/Desktop/ECM_DEMO$ sudo ./ECM_DEMO -t down

  Note: Users can link libecmcall.so into their own programs separately, and no need to use ECM_DEMO/ECM_DEMO_AUTO.

2. How to generate static link library files
   a).Enter demo code file directory.
    user@ubuntu:~/Desktop/ECM_DEMO$ 
   b).Do make
    user@ubuntu:~/Desktop/ECM_DEMO$ sudo make Aslibrary
   c).Then generate file:ECM_DEMO, ECM_DEMO_AUTO and libecmcall.a, ECM_DEMO and ECM_DEMO_AUTO is the executable file, libecmcall.a is the static link library file.

   User can call the file ECM_DEMO and ECM_DEMO_AUTO directly. Example:

   user@ubuntu:~/Desktop/ECM_DEMO$ sudo ./ECM_DEMO -t up

  Note: Users can integrate libecmcall.a into their own programs separately, and no need to use ECM_DEMO/ECM_DEMO_AUTO.

3. command sets

Example:

  ECM_DEMO -t up

  ECM_DEMO -t status

  ECM_DEMO -t down

  ECM_DEMO -t up -p /dev/ttyUSB1 -a 3gnet

  ECM_DEMO -t status -p /dev/ttyUSB1

  ECM_DEMO -t down -p /dev/ttyUSB1
  
  
  Automatically connect:
  
  ECM_DEMO_AUTO -p /dev/ttyUSB1
  
  ECM_DEMO_AUTO -a 3gnet -p /dev/ttyUSB1
