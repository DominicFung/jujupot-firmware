// extern int AWS_STATUS;
// const int AWS_UNCONNECTED = 0;
// const int AWS_CONNECTED = 1;

void aws_connect();
void await_get_shadow();
void update_shadow(char *payload);
void keep_alive();

// void aws_send(const std::string& s);
// void aws_send_with_retry(const std::string& s);

//std::string aws_get_shadow();

// extern std::string aws_shadow_topic_prefix;
// void getDeviceDataCallback(char *topicName, int payloadLen, char *payLoad);
// void updateDeviceDataCallback(char *topicName, int payloadLen, char *payLoad);
