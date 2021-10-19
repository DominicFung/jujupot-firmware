#include <Arduino.h>
#include <string>

#include <AWS_IOT.h>
#include "awscomm.h"

#include "secret_aws.h"

AWS_IOT aws;

// AWS IoT core shadow topics
// char aws_topic_prefix[]="$aws/things/";
std::string aws_topic_prefix = std::string("$aws/things/") + deviceId;

std::string aws_topic_get = aws_topic_prefix + "/shadow/get";
std::string aws_topic_get_accepted = aws_topic_prefix + "/shadow/get/accepted";
std::string aws_topic_get_rejected = aws_topic_prefix + "/shadow/get/rejected";

std::string aws_topic_update = aws_topic_prefix + "/shadow/update";
std::string aws_topic_update_accepted = aws_topic_prefix + "/shadow/update/accepted";
std::string aws_topic_update_rejected = aws_topic_prefix + "/shadow/delete/rejected";

/**
 * msgReceived statuses:
 *    - 0: no message
 *    - 1: UNKNOWN message origin.
 * 
 *    - 2: get shadow accept
 *    - 3: get shadow reject
 * 
 *    - 4: update shadow accept
 *    - 5: update shadow reject
 */
int msgReceived=0, msgCount=0;
char rcvdPayload[1024];

void cbhandler(char *topicName, int payloadLen, char *payLoad)
{
  
  msgReceived = 1;
  if (0==strcmp(topicName, aws_topic_get_accepted.c_str())) {
    msgReceived = 2;
  } else if (0==strcmp(topicName, aws_topic_get_rejected.c_str())) {
    msgReceived = 3;
  } else if (0==strcmp(topicName, aws_topic_update_accepted.c_str())) {
    msgReceived = 4;
  } else if (0==strcmp(topicName, aws_topic_update_rejected.c_str())) {
    msgReceived = 5;
  }

  rcvdPayload[payloadLen] = 0;
  strncpy(rcvdPayload, payLoad, payloadLen);
  
  Serial.print("cbhandler: ");
  Serial.println(rcvdPayload);
}

void aws_connect()
{
  if(aws.connect(HOST_ADDRESS,CLIENT_ID)== 0)
  {
      Serial.println("Connected to AWS");
      delay(1000);

      char all_success='y';

      // if(0==aws.subscribe(TOPIC_NAME, cbhandler)) {
      //     Serial.print("Subscribe Successfull to ");
      //     Serial.println(TOPIC_NAME);
      // } else {
      //     Serial.print(" --- FAILED to subscribe: ");
      //     Serial.print(TOPIC_NAME);
      //     Serial.println(" --- ");
      //     all_success='n';
      // }

      char get_topic_accept[aws_topic_get_accepted.length()+1];
      strcpy(get_topic_accept, aws_topic_get_accepted.c_str());
      if (0==aws.subscribe(get_topic_accept, cbhandler)) {
          Serial.print("Subscribe Successfull to ");
          Serial.println(get_topic_accept);
      } else {
          Serial.print(" --- FAILED to subscribe: ");
          Serial.print(get_topic_accept);
          Serial.println(" --- ");
          all_success='n';
      }

      char get_topic_reject[aws_topic_get_rejected.length()+1];
      strcpy(get_topic_reject, aws_topic_get_rejected.c_str());
      if (0==aws.subscribe(get_topic_reject, cbhandler)) {
          Serial.print("Subscribe Successfull to ");
          Serial.println(get_topic_reject);
      } else {
          Serial.print(" --- FAILED to subscribe: ");
          Serial.print(get_topic_reject);
          Serial.println(" --- ");
          all_success='n';
      }

      char update_topic_accept[aws_topic_update_accepted.length()+1];
      strcpy(update_topic_accept, aws_topic_update_accepted.c_str());
      if (0==aws.subscribe(get_topic_reject, cbhandler)) {
          Serial.print("Subscribe Successfull to ");
          Serial.println(update_topic_accept);
      } else {
          Serial.print(" --- FAILED to subscribe: ");
          Serial.print(update_topic_accept);
          Serial.println(" --- ");
          all_success='n';
      }

      char update_topic_reject[aws_topic_update_rejected.length()+1];
      strcpy(update_topic_reject, aws_topic_update_rejected.c_str());
      if (0==aws.subscribe(get_topic_reject, cbhandler)) {
          Serial.print("Subscribe Successfull to ");
          Serial.println(update_topic_reject);
      } else {
          Serial.print(" --- FAILED to subscribe: ");
          Serial.print(update_topic_reject);
          Serial.println(" --- ");
          all_success='n';
      }

      if (all_success=='n') {
          Serial.println("Subscribe Failed, Check the Thing Name and Certificates");
          while(1);
      }
  }
  else
  {
      Serial.println("AWS connection failed, Check the HOST Address");
      while(1);
  }
}

void aws_send(const std::string& s) {
  int n = s.length();
  char payload[n+1];

  Serial.print("s: ");
  Serial.println(s.c_str());
  
  strcpy(payload, s.c_str());

  Serial.print("payload: ");
  Serial.println(payload);

  Serial.print(" -- Sending to AWS IoT from ESP32, count: ");
  Serial.println(msgCount);
  msgCount++;

  // old: aws.publish(TOPIC_NAME, payload) == 0
  char topic[aws_topic_update.length()+1];
  strcpy(topic, aws_topic_update.c_str());

  if(aws.publish(topic, payload) == 0)
  {        
      Serial.print("Publish Message: ");
      Serial.println(payload);
      msgCount++;
  }
  else
  {
      Serial.println("Publish failed");
  }
}

std::string aws_get_shadow() {
  char topic[aws_topic_get.length()+1];
  strcpy(topic, aws_topic_get.c_str());

  int tick=0, RETRY=3;

  while (tick < RETRY) {
    if (aws.publish(topic, "") == 0) {
      tick=0;
      while (tick < 10) {
        if (msgReceived == 1) {
          
          
          Serial.println(rcvdPayload);
          msgReceived = 0;
        } else if (msgReceived == 2) {
          

          msgReceived = 0;
        } else {
          delay(1000);
          tick++;
        }
      }
    } else {
      tick ++;
      delay(1000);
    }
  }
}

