    Copyright (c) 1998  Microsoft Corporation
    Module Name: IpArp.exe    
    File       : ReadMe.txt
    Description: This sample demonstrates the use of IP Helper functions
  		 to manipulate ARP cache.
    Author:
    Frank Li                 April 16, 1998
            
    Revision History:
    Who         When         What
    --------    --------     ---------------------------------
    Frank Li    04-16-98     created   


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
 

-------
Usage:
-------
IpArp -s inet_addr eth_addr [if_addr]
IpArp -d inet_addr [if_addr]
IpArp -a
-a            Displays current ARP entries by interrogating the current
              protocol data.
-d            Deletes the host specified by inet_addr.
-s            Adds the host and associates the Internet address inet_addr
              with the Physical address eth_addr.  The Physical address is
              given as 6 hexadecimal bytes separated by hyphens. The entry
              is permanent.
eth_addr      Specifies a physical address.
if_addr       If present, this specifies the Internet address of the
              interface whose address translation table should be modified.
              If not present, the first applicable interface will be used.
Example:
   >IpArp -s 157.55.85.212   00-aa-bb-cc-dd-ee 0x2000003 .... Add a static
                                  arp entry on interface number 0x2000003.
   >IpArp -a                                    ....Displays the arp table.
   >IpArp -d 157.55.85.212                      ....Delete an entry.
