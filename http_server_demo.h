#ifndef HTTP_serverDemo_H
#define HTTP_serverDemo_H

typedef enum
{
    MAIN_PAGE=0,
    TOGGLE_LED_A,
    TOGGLE_LED_B,
    FAVICON,
    BAD_REQUEST
}webRequest_t;

void DEMO_HTTP_Server(void);
webRequest_t getRequestType(char*);

#endif