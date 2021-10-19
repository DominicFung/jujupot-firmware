#include <Arduino.h>
#include <string>

extern int msgReceived;
extern char payload[1024];
extern char rcvdPayload[1024];

void aws_connect();
void aws_send(const std::string& s);
std::string aws_get_shadow();

// void cbhandler(char *topicName, int payloadLen, char *payLoad);