The vbtcp sample can run as either an echo server or an echo client over TCP.

First run vbtcp as the echo server, select the server type on startup, and click listen. Then either on the same machine or a remote machine, run another vbtcp as the echo client. In the client, specify the server machine name or its IP address and click Connect (by default both server and client use port 5000). Once the connection is established, the client can send messages to the server and display the echos from the server.
