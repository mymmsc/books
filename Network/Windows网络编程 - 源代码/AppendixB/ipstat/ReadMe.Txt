    Copyright (c) 1998  Microsoft Corporation
    Module Name: IpStat.exe    
    File       : ReadMe.txt
    Description: This sample demonstrates the use of IP Helper APIs to
                 get IP statistics.
    Author:
    Frank Li            April 21, 1998
    
    Revision History:
    Who         When         What
    --------    --------     ---------------------------------
    Frank Li    04-21-98     created   


---------------------
Supported Platforms:
---------------------
Windows NT 5.0, Windows NT 4.0 with SP4, and Windows 98.


---------------------
Building this sample:
---------------------
To Build the sample application, Microsoft Visual C++ 5.0 
with Service Pack 3 or later should have been installed:

  nmake cleanall
  nmake

------
Usage:
------
Manipulates IP Statistics.

IpStat -p proto      Shows connections for the protocol specified
                     by proto, proto may be tcp or udp.
IpStat -s [-p proto] Displays per-protocol statistics.
                     By default, statistics are shown for
                     IP, ICMP, TCP and UDP; the -p option
                     may be used to specify a subset of the default.
Examples:

> IpStat -p tcp
> IpStat -s
