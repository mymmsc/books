The vbudp sample can send or receive UDP datagrams and broadcasts.

To send and receive UDP datagrams, start the first instance of vbudp on machine1, and the second instance of vbudp on machine2. In vbudp 1, specify machine2's name or IP, remote port 6000, local port 5000, click bind. In vbudp 2, specify machine1's name or IP, remote port 5000, local port 6000, click bind. Then the two vbudp instance can communicate with each other.

To send and receive UDP broadcase, start the first instance of vbudp on machine1, and the second instance of vbudp on machine2. In vbudp 1, specify 255.255.255.255, remote port 6000, local port 5000, click bind. In vbudp 2, specify 255.255.255.255, remote port 5000, local port 6000, click bind. Then the two vbudp instance can send broadcast to each other.

