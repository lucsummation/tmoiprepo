/*
	udp server, listen to the tmoip packets sent from the receiver by the ethernet.
*/
#include<stdio.h>	
#include<string.h> 	
#include<stdlib.h> 	
#include <unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include "tmoip.h"

/**
 *  @brief main function will open the socket, etc. and waiting to receive the UDP data 
 */
int main(void)
{
	struct sockaddr_in server, client;
	
	int s, i, slen = sizeof(client) , recv_len;
	char buf[UDP_packet_max_len];

	//create a UDP socket
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		error_msg_print("Could not create socket.");
	}
	printf("Socket created.\n");
	
	// zero out the structure
	memset((char *) &server, 0, sizeof(server));
	
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//bind socket to port
	if( bind(s , (struct sockaddr*)&server, sizeof(server) ) == -1)
	{
		error_msg_print("Bind failed.");
	}
	printf("Bind done.\n");

	//keep listening for data
	while(1)
	{
		printf("\nWaiting for data...\n");
		fflush(stdout);
		
		//clear the buffer by filling null, it might have previously received data.
		memset(buf, '\0', UDP_packet_max_len);		
		
		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(s, buf, UDP_packet_max_len, 0, (struct sockaddr *) &client, &slen)) == -1)
		{
			error_msg_print("recvfrom() failed.");
		}
		
		//print details of the client/peer and the data received
		printf("Received packet from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
		//printf("Data: %s\n" , buf);
		
   		if (recv_len >= TMoIP_RawData_min_len)  // The minimum TMoIP raw packet size = 1
        	{
                	// parse the received tmoip data.
                	paser_recvd_tmoip_data(buf, recv_len);
			
        	}


		//now reply the client with the same data
		if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &client, slen) == -1)
		{
			error_msg_print("sendto() failed.");
		}
	}

	close(s);
	return 0;
}


// print out the error message.
void error_msg_print(char *s)
{
	perror(s);
	exit(1);
}
