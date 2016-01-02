// File: prg7_2.c
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

main(int argc, const char **argv)
{
     ulong_t addr;
     struct hostent *hp;
     char **p;
     if (argc != 2) {
        (void) printf("usage: %s IP-address\n", argv[0]);
        exit (1);
        }
     if ((int)(addr = inet_addr(argv[1])) == -1) {
        (void) printf("IP-address must be of the form a.b.c.d\n");
        exit (2);
        }
     hp = gethostbyaddr((char *)&addr, sizeof (addr), AF_INET);
     if (hp == NULL) {
        (void) printf("host information for %s not found\n", argv[1]);
        exit (3);
        }
     for (p = hp->h_addr_list; *p != 0; p++) {
        struct in_addr in;
        char **q;
        (void) memcpy(&in.s_addr, *p, sizeof (in.s_addr));
        (void) printf("%s\t%s", inet_ntoa(in), hp->h_name);
        for (q = hp->h_aliases; *q != 0; q++)
                  (void) printf(" %s", *q);
        (void) putchar('\n');
        }
     exit (0);
}
