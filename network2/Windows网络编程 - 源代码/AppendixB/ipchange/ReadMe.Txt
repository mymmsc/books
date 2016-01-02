    This sample illustrates how to programmatically change an IP address for 
    a specific network adapter on your machine.  This program also demonstrates 
    how to retrieve existing network adapter IP configuration information.

    IP configurations can be changed for a specific network adapter by using the
    AddIpAddress() and DeleteIpAddress() Ip Helper API functions.  These functions 
    require you to understand adapter index numbers and IP context numbers.  In 
    Windows, every network adapter has a unique index ID and every IP address has 
    a unique context ID. Adapter index IDs and IP context numbers can be retrieved 
    using the GetAdaptersInfo() IP Helper function.  This program features a list 
    option that displays current network adapter configuration information by 
    showing all adapters index numbers and IP address context numbers associated 
    with their corresponding network adaptors.

    To execute this application, simply build the application using the Visual C++
    (Version 5 SP 3 or better) Nmake.exe program generation facility.  An ipchange.exe
    program should result.  Execute the ipchange.exe with the following parameters:

    Ipchange.exe [ -l ] [ -a -n<index id> -i<ip address> -m<subnet mask> ]
                 [ -d -c<context id>]

        -l List adapter index IDs and IP Address context ID information
        -a Add IP Address option
        -d Delete IP Address option
        -i IP Address to specify with -a option
        -m Subnet Mask to specify with -a option
        -c IP context ID for an existing IP address
        -n Index ID of an existing network adapter

Platforms:

Windows NT 5.0