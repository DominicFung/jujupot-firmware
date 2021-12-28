##
# This script writes the following certs:
#    -- certs/aws-root-ca.pem
#    -- certs/certificate.pem.crt
#    -- certs/private.pem.key
# to the following file: .pio/libdeps/az-delivery-devkit-v4/AWS_IOT/src/
#
#  PLEASE NOTE:
#     We override the WHOLE file. 

import os

path_to_c_file = ".pio/libdeps/az-delivery-devkit-v4/AWS_IOT/src/"
c_file = "aws_iot_certficates.c"

if not os.path.isfile("../certs/aws-root-ca.pem"):
  print("ERROR: certs/aws-root-ca.pem not found.")
  exit(1)

if not os.path.isfile("../certs/certificate.pem.crt"):
  print("ERROR: certs/certificate.pem.crt not found.")
  exit(1)

if not os.path.isfile("../certs/private.pem.key"):
  print("ERROR: certs/private.pem.key not found.")
  exit(1)

eol_car = "\\n\\\n"

with open("../certs/aws-root-ca.pem") as file:
  aws_root_ca = file.read().replace("\n", eol_car)

with open("../certs/certificate.pem.crt") as file:
  certificate_pem = file.read().replace("\n", eol_car)

with open("../certs/private.pem.key") as file:
  private_pem = file.read().replace("\n", eol_car)

aws_root_ca_template = f'const char aws_root_ca_pem[] = {{"{aws_root_ca}"}};\n'
certificate_pem_template = f'const char certificate_pem_crt[] = {{"{certificate_pem}"}};\n'
private_pem_template = f'const char private_pem_key[] = {{"{private_pem}"}};\n'

aws_iot_certificates_template_start = """/*
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * Additions Copyright 2016 Espressif Systems (Shanghai) PTE LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

/**
 * @file aws_iot_certifcates.c
 * @brief File to store the AWS certificates in the form of arrays
 */

#ifdef __cplusplus
extern "C" {
#endif

"""

aws_iot_certificates_template_end = """


#ifdef __cplusplus
}
#endif
"""


f = open(path_to_c_file+c_file, "w")
f.write(
  aws_iot_certificates_template_start+
  aws_root_ca_template+
  certificate_pem_template+
  private_pem_template+
  aws_iot_certificates_template_end
)
f.close()

print("  -- Load Cert Complete.")