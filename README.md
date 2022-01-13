# Jujupots - ESP32-WROOM (Arduino)

We're using PlatformIO for library management.

## IOT Certificate
This project is powered by [Hommie.io](https://hommie.io)

Go to the [Hommieo Admin portal](https://makers.hommie.io) and download a hommieo Cert Zip file.
Then run the following:
```
python3 init_product.py -i "../../Downloads/hommieo_********.zip"
```
This writes all the required configuration parameters to our h files. You are ready to begin testing your product!

<br>

---
## Manual configurations

---

Go to the following C file:

```
.pio/libdeps/az-delivery-devkit-v4/AWS_IOT/src/aws_iot_certificates.c
```

update the following:
- aws_root_ca_pem
- certificate_pem_crt
- private_pem_key