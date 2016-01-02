    This sample illustrates how to programmatically release and renew IP addresses
    obtained through DHCP.  This program also demonstrates how to retrieve existing
    network adapter configuration information.

    IP address assignment from DHCP can be updated using the IpRenewAddress() and
    IpReleaseAddress() IP Helper API functions.  These functions require you to 
    understand adapter index numbers.  In Windows, every network adapter has a 
    unique index ID.  Adapter index IDs can be retrieved using the GetAdaptersInfo() 
    IP Helper function.  This program features a list option that displays current 
    network adapter configuration information showing all adapters index numbers 
    associated with their corresponding network adaptors.

    To execute this application, simply build the application using the Visual C++
    (Version 5 SP 3 or newer) Nmake.exe program generation facility.  An iprenew.exe
    program should result.  Execute the iprenew.exe with the following parameters:

    Iprenew.exe [ -l ] [ -r<index id> ] [ -n<index id> ]

        -l List adapters with corresponding index ID information
        -r Release IP address for adapter index ID
        -n Renew IP address for adapter index ID

Platforms:

Windows NT 5.0, Windows 98