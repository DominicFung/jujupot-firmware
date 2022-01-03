#include <AWS_IOT.h>
#include <WiFi.h>

#include "pot/potshadow.h"
#include "wifi/wifiutil.h"
#include "aws/awscomm.h"

#include "secret_aws.h"

char productId[] = "58109219-d923-49fc-b349-d713f2c7d2a3";
char verificationId[] = "204ed6d7-efb4-4b55-99f1-50704d984219";
char potType[] = "small-planter-v1";
char color[] = "blue-white-v1";
char plant[] = "sedum-morganianum_costco_pid";
int fwVersion = 1;

int tock = 0;

void setup() {
    Serial.begin(9600); // 115200 by default
    delay(2000);

    xTaskCreatePinnedToCore(
      keepWiFiAlive,
      "keepWiFiAlive",  // Task name
      5000,             // Stack size (bytes)
      NULL,             // Parameter
      0,                // Task priority
      NULL,             // Task handle
      0 /* Core where the task should run */
    );

    aws_connect();
    await_get_shadow();

    init_shadow(productId, fwVersion);
    add_device_id(CLIENT_ID);
    add_signoff(verificationId);


    unsigned long time = get_time();
    std::string output = get_new_shadow(time);

    int n = output.length();
    char payload2[n+1];
    strcpy(payload2, output.c_str());

    update_shadow(payload2);
}

void loop() {
    if(tock >= 5) {
        tock=0;
        keep_alive();
    }  
    vTaskDelay(12 * 1000 / portTICK_RATE_MS); 
    tock++;
}