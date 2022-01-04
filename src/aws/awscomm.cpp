#include <Arduino.h>
#include <string>

#include <AWS_IOT.h>
#include "awscomm.h"

#include "secret_aws.h"
#include "awstopics.h"

AWS_IOT aws;

int tick=0,msgCount=0,msgReceived = 0;
char payload[512];
char rcvdPayload[512];
char _empty_payload[] = "";

void dataCallback(char *topicName, int payloadLen, char *payLoad) {
    strncpy(rcvdPayload, payLoad, payloadLen);
    rcvdPayload[payloadLen] = 0;

    // Note: *topicName is not null-terminated \0
    if (strncmp(topicName, aws_shadow_topic_get_return, strlen(aws_shadow_topic_get_return)-1) == 0) {
        msgReceived = 1;
        Serial.println("Received getDeviceDataCallback:");
        Serial.println(payLoad);
    } else if (strncmp(topicName, aws_shadow_topic_update_return, strlen(aws_shadow_topic_update_return)-1) == 0) {
        msgReceived = 2;
        Serial.println("Received updateDeviceDataCallback:");
        Serial.println(payLoad);
    } else {
      Serial.println("Received unknown topic.");
    }
}

void await_get_shadow() {
  int retry = 0;
  int max_retry = 20;

  while (retry < 5) {
    if (aws.publish(aws_shadow_topic_get, _empty_payload) == 0) {
      Serial.println("Publish Get OK.");
      break;
    } else {
      Serial.println("Publish Get Failed.");
    }
    retry++;
    delay(1000);
  }

  retry = 0;
  while(retry < max_retry) {
    if(msgReceived == 1) {
        msgReceived = 0;
        Serial.print("Received Message:");
        Serial.println(rcvdPayload);
        return;
    }
    retry++;
    delay(1000);
  }

  Serial.println("Max retry reached, GetShadow Failed.");
}

void update_shadow(char *payload) {
  if(aws.publish(aws_shadow_topic_update, payload) == 0) {        
      Serial.println("Publish Update Ok.");
  } else {
      Serial.println("Publish Update Failed.");
  }
}

void aws_connect() {
  Serial.println("Connected to wifi");
  if(aws.connect(HOST_ADDRESS, CLIENT_ID)== 0) {
      Serial.println("Connected to AWS");
      delay(1000);

      if(0==aws.subscribe(aws_shadow_topic_get_return, dataCallback)){
          Serial.println("Subscribe getDevice Successfull");
      } else {
          Serial.println("Subscribe Get Failed, Check the Thing Name and Certificates");
          while(1);
      }

      if(0==aws.subscribe(aws_shadow_topic_update_return, dataCallback)){
          Serial.println("Subscribe updateDeviceData Successfull");
      } else {
          Serial.println("Subscribe Update Failed, Check the Thing Name and Certificates");
          while(1);
      }
  } else {
      Serial.println("AWS connection failed, Check the HOST Address");
      while(1);
  }
  delay(2000);
}

void keep_alive() {
  if(aws.publish(aws_custom_ping, "") == 0) {
        Serial.println("Keep Alive OK");
    } else {
        Serial.println("Keep Alive Failed");
    }
}