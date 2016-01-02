To run the namepipe sample, start the pipe server first, and then start the pipe client to connect and send a test message to the server.

The namepipe server demonstrated in the server only takes one client pipe connection. Due to no support of threads in VB (VB runtime is not threadsafe), it's not easy to implement a name pipe server that can take multiple client pipe connections. It's still possible to write a name pipe server in VB that can take multiple pipe clients and does not block by using overlapped I/O and controling the timeout in WaitForMultipleObject. Serious scalable server development however should utilize C/C++ and the I/O completion port on Windows NT.

