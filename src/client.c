

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

char* read_data() {

                char *ret_buffer = (char *)malloc(1023);

                fgets(ret_buffer, sizeof(ret_buffer), stdin);

                return ret_buffer;
}

int main(int argc, char ** argv) {

                int i, j = 0, sd, res, addrlen_server, error, counter;
                uint32_t result;

                //connector's adress information
                struct sockaddr_in serveraddr;
                struct hostent *server;

                //struct hostent *hp, *ǵethostbyname();
                char buffer[1024], *mode, *anzahl;
                char *port, *host;
                unsigned char requestid = '0';


                if (argc < 2)
                {
                               printf("Falsche eingabe\n");
                               printf("Aufruf:\n");
                               printf("%s [host] [port]\n\n", argv[0]);
                               return -1;
                }

                port = argv[2];
                host = argv[1];

                sd = socket(AF_INET, SOCK_STREAM, 0);

                //sd = socket(host_info->ai_family, host_info->ai_socktype, host_info->ai_protocol);

                if (sd < 0)
                {
                               printf("Cannot create datagram socket\n");
                               return -1;
                }
                ;

                //hp = gethostbyname(argv[1]);

                bzero((char *)&serveraddr, sizeof(serveraddr));

                //host byte order
                serveraddr.sin_family = AF_INET;
                serveraddr.sin_port = htons(atoi(port));
                //serveraddr.sin_addr = argv[1];


                printf("Host: %s\n", argv[1]);
                printf("Port: %s\n", argv[2]);


                server = gethostbyname(argv[1]);

                if (server == NULL)
                {
                               printf("ERROR, no such host");
                              exit(0);
                }

                bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);

                if (connect(sd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0) {
                               printf("ERROR connecting\n");
                               exit(0);
                }

                while(1)
                {

                               bzero(buffer, sizeof(buffer));
                               counter = 0;

                               //Daten einlesen

                               printf("Bitte gewuenschten Modus angeben.\n");
                               printf("0 - Summe eingegebener Zahlen bilden\n");
                               printf("1 - Anzahl eingegebener Zahlen bestimmen\n");
                               printf("2 - Server stoppen\n");
                               printf("3 - Programm beenden\n");

                               //RequestID

                               buffer[0] = ++requestid;
                               counter = counter + (int) sizeof(buffer[0]);        //counter für später senden, wie lang ist der string?

                               //Anzahl
                               mode = read_data();
                               buffer[1] =  mode[0] - 48; //mode in hex wandeln und in Buffer schreiben

                               counter = counter + (int) sizeof(buffer[1]);

                               //Zahlen einlesen
                               if ( mode[0] == '0' || mode[0] == '1' ) {
                                               printf("Wie viele Zahlen sollen eingegeben werden?\n");

                                               anzahl = read_data();

                                               if ( atoi(anzahl) != 0 && atoi(anzahl) <= 12 ) {

                                                               buffer[2] = (char) atoi(anzahl); //in Buffer schreiben
                                                               counter = counter + (int) sizeof(buffer[2]);

                                                               for( i = 0; i < atoi(anzahl); i++) {

                                                                              printf("Zahl %d:\n", i+1);
                                                                              char* test = read_data();
                                                                              uint32_t eingabe = atoi(test);
                                                                              memcpy(buffer + (3 + (i * 4)),  &eingabe, sizeof(uint32_t));
                                                                              counter = counter + (int) sizeof(uint32_t);
                                                               }
                                               }
                               }
                               else if ( mode[0] == '3' )
                               {
                                               return 0;
                               }

                               //sende Daten
                               printf("---------------------------------------------------------------\n");
                               printf("Sende Daten:\n");
                               printf("RequestID: %c\n", requestid);
                               printf("Function: %c\n", mode[0]);

                               //res = sendto(sd, buffer, sizeof(buffer), 0, (struct sockaddr*) &serveraddr, sizeof(serveraddr));

                               printf("Counter: %d, Strlen: %zu\n", counter, strlen(buffer));

                               res = write(sd, buffer, counter);

                               if (res < 0)
                               {
                                               printf("Error sending datagram\n");
                                               return -1;
                               }

                               printf("gesendet\n");

                               //res = recefv(sd, buffer, sizeof(buffer), 0, (struct sockaddr*) &serveraddr, &addrlen_server);

                               res = read(sd, buffer, sizeof(buffer));

                               if (res<0) {
                                               printf("Error reading datagram\n");
                                               return -1;
                               }

                               printf("---------------------------------------------------------------\n");
                               printf("Empfangene Daten:\n");
                               printf("RequestID: %c\n", buffer[0]);
                               printf("Function: %02X\n", buffer[1]);
                               printf("%06x\n", buffer);
                               memcpy(&result, buffer + 2, sizeof(uint32_t));
                               printf("RESULT: %d\n", (int) ntohl(result));

                }
                close(sd);


                return(0);
}

