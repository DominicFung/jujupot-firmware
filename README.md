# JUJU Pots - ESP32-WROOM (Arduino)

We're using PlatformIO for library management.

## IOT Certificate

Go to Jujupot Admin portal and download jujupot-certs.zip
Then run the following:
```
python3 init_product.py -i "../../Downloads/jujupot-certs (17).zip"
```
This writes all the required configuration parameters to our h files.

---

## Manual Certificate loading (automated by configure_certs.py)
Go to the following C file:

```
.pio/libdeps/az-delivery-devkit-v4/AWS_IOT/src/aws_iot_certificates.c
```

update the following:
- aws_root_ca_pem
- certificate_pem_crt
- private_pem_key