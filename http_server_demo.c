#include <stdio.h>
#include <string.h>
#include "mcc_generated_files/mcc.h"
#include "http_server_demo.h"
#include "mcc_generated_files/TCPIPLibrary/tcpv4.h"
#include "mcc_generated_files/TCPIPLibrary/ipv4.h"
#include "mcc_generated_files/TCPIPLibrary/tcpip_config.h"

char* expected_requests[4] = {"GET / HTTP/1.1","GET /?TA=LED_A","GET /?TB=LED_B","GET /favicon.i"};
char main_page_content[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 398\r\n\r\n<!DOCTYPE html><html><body><form name=input method=get><table align=center width=500 bgcolor=Red border=4><tr><td align=center colspan=2><font size=7 color=white face=verdana><b>LED CONTROL</b></font></td></tr><tr><td align=center bgcolor=Blue><input name=TA type=submit value=LED_A></td><td align=center bgcolor=Green> <input name=TB type=submit value=LED_B></td></tr></table></form></body></html>";
char not_modified_response[] = "HTTP/1.1 304 Not Modified\r\n\r\n";
char bad_request_response[] = "HTTP/1.1 404 Bad Request\r\n\r\n";

//Implement an echo server over TCP
void DEMO_HTTP_Server(void)
{
    // create the socket for the TCP Server
    static tcpTCB_t port80TCB;

    // create the RX Server's buffers    
    static uint8_t rxdataPort80[800];
    
    uint16_t rxLen, k;
    socketState_t socket_state;

    socket_state = TCP_SocketPoll(&port80TCB);
    
    webRequest_t request;

    switch(socket_state)
    {
        case NOT_A_SOCKET:
            // Inserting and initializing the socket
            TCP_SocketInit(&port80TCB);
            break;
        case SOCKET_CLOSED:
            //configure the local port
            TCP_Bind(&port80TCB, 80);
            // add receive buffer
            TCP_InsertRxBuffer(&port80TCB, rxdataPort80, sizeof(rxdataPort80));
            // start the server
            TCP_Listen(&port80TCB);
            break;
        case SOCKET_CONNECTED:
            // check if the buffer was sent, if yes we can send another buffer
            if(TCP_SendDone(&port80TCB))
            {
                // check to see  if there are any received data
                rxLen = TCP_GetRxLength(&port80TCB);
    
                if(rxLen > 0)
                {
                    webRequest_t request = getRequestType((char*)&rxdataPort80);
                    switch (request)
                    {
                        
                        case MAIN_PAGE:
                              TCP_Send(&port80TCB, (uint8_t*)main_page_content, sizeof(main_page_content));                            
                              break;
                        case TOGGLE_LED_A:
                              RD0_Toggle();                             
                              TCP_Send(&port80TCB, (uint8_t*)not_modified_response, sizeof(not_modified_response));
                              break;                             
                        case TOGGLE_LED_B:
                              RD1_Toggle(); 
                              TCP_Send(&port80TCB, (uint8_t*)not_modified_response, sizeof(not_modified_response));
                              break;
                        case FAVICON:
                              TCP_Send(&port80TCB, (uint8_t*)not_modified_response, sizeof(not_modified_response));
                              break;
                        case BAD_REQUEST:
                              TCP_Send(&port80TCB, (uint8_t*)bad_request_response, sizeof(bad_request_response));
                              break;
                        default:
                              break;
                    }
                    TCP_InsertRxBuffer(&port80TCB, rxdataPort80, sizeof(rxdataPort80)); 
                                         
                }            
            }
            break;
        case SOCKET_CLOSING:
            TCP_SocketRemove(&port80TCB);
            break;
            
        default:
            // we should not end up here
            break;
    }
}


webRequest_t getRequestType(char* rx_Data)
{
      uint8_t i;     
      char rx_Request[15];
      
      memcpy(rx_Request, rx_Data,sizeof(rx_Request)-1);
      rx_Request[sizeof(rx_Request)-1]='\0';
      
      for(i=0;i<4;i++)
      {
          if(strcmp(rx_Request, expected_requests[i])==0)
              return i;
      }
      
      return BAD_REQUEST;
}
