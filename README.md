# JUJU Pots - ESP32-WROOM (Arduino)

We're using PlatformIO for library management.

## IOT Certificate

Go to the following C file:

```
.pio/libdeps/az-delivery-devkit-v4/AWS_IOT/src/aws_iot_certificates.c
```

update the following:
- aws_root_ca_pem
- certificate_pem_crt
- private_pem_key
