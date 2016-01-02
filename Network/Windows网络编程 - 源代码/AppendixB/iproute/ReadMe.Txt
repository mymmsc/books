    Module Name: IpRoute.exe   
    File       : ReadMe.txt
    Description: This sample demonstrates the use of IP Helper API
                 functions to manipulate the routing table.
    Author:
    Frank Li           April 18, 1998
                
    Revision History:
    Who         When         What
    --------    --------     ---------------------------------
    Frank Li    04-18-98     created   


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
Manipulates network routing tables.

IpRoute -p                                               ...Prints route table.
IpRoute -a destination netmask gateway interface [metric]...Adds a route.
IpRoute -d destination                                   ...Deletes routes to
                                                            destination.

destination  Specifies the destination host.

netmask      Specifies a subnet mask value to be associated
             with this route entry.

gateway      Specifies gateway.

interface    Specifies the interface ip.

metric       The cost for this destination

Diagnostic Notes:

Invalid MASK generates an error, that is when (DEST & MASK) != DEST.
Example> IpRoute -a 157.0.0.0 155.0.0.0 157.55.80.1 157.55.80.9
         IpRoute: Invalid Mask 155.0.0.0

Examples:

> IpRoute -p
> IpRoute -a 157.0.0.0    255.0.0.0 157.55.80.1  157.55.80.9           1
             ^destination ^mask     ^gateway     ^existing interface   ^metric
> IpRoute -p
> IpRoute -d 157.0.0.0
> IpRoute -p

