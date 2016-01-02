The vbtos sample demonstrates setting IP_TOS option to a UDP socket. 
This sample doesn't have a user interface. It displays a VB message
box to indicate the state of the program.

You need to verify that the IP TOS bit was set by either using Netmon
or the SIO_RCVALL sample which can filter for specified protocols and
port number.