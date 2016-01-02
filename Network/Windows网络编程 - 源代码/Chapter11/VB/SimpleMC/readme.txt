The vbmcast sample demonstrates sending and receiving muliticast packet.

Click Bind and Join MC, you will join the MC group specified and are able 
to send and receive multicast packets.

Note that this is a simple sample and is targeted for non multihomed machines.
For this sample to work properly on multihomed machines a call to setsocktopt
and IP_MULTICAST_IF should be made to explicitly tell which interface should
be used for sending and receiving multicast data.