##
# PRE-REQ:
#   - hommieo-certs.zip file from Hommieo Admin site.
#
# That zip folder contains the following:
#  - certs/aws-root-ca.pem
#  - certs/certificate.pem.crt
#  - certs/private.pem.key
#  - configuration.json


#  - secrets_aws.h
#  - product_config.h
#
# OPTIONS:
#  --input -i <input>    ::  default (../hommieo-certs.zip)
#  --output -o <output>  ::  default (./downloads)

import os
import json
import sys
import glob

from optparse import OptionParser
import zipfile

if not os.path.exists('./certs'):
    os.makedirs('./certs')

parser = OptionParser()
parser.add_option("-i", "--input", 
  dest="input", default="hommieo-certs.zip",
  help="Input file to read from. Default: hommieo-certs.zip"
)

parser.add_option("-o", "--output",
  dest="output", default="./downloads",
  help="Output file to write to. Default: ./downloads"
)

(options, args) = parser.parse_args()
with zipfile.ZipFile(options.input, 'r') as zip_ref:
  zip_ref.extractall(options.output)

print(" -- Unzip Successful -- ")

if not os.path.isfile(options.output + "/aws-root-ca.pem"):
  print(" EXIT: aws-root-ca.pem not found in " + options.output)
  sys.exit(1)

if not os.path.isfile(options.output + "/device.crt"):
  print(" EXIT: device.crt not found in " + options.output)
  sys.exit(1)

if not os.path.isfile(options.output + "/device.key"):
  print(" EXIT: device.key not found in " + options.output)
  sys.exit(1)

if not os.path.isfile(options.output + "/device.json"):
  print(" EXIT: device.json not found in " + options.output)
  sys.exit(1)

# distruibute the certs
os.rename(options.output + "/aws-root-ca.pem", "./certs/aws-root-ca.pem")
os.rename(options.output + "/device.crt", "./certs/certificate.pem.crt")
os.rename(options.output + "/device.key", "./certs/private.pem.key")

print(" -- Successfully Write Certs -- ")

# write the .h files
config = {}
with open(options.output + "/device.json", 'r') as c:
  config = json.load(c)
  c.close()

aws_secret = '''// THIS FILE IS AUTOGENERATED. DO NOT EDIT.
#pragma once

#ifndef SECRET_AWS_H
  #define SECRET_AWS_H
  #define HOST_ADDRESS "{iotEndpoint}"
  #define CLIENT_ID  "{clientId}"
  #define DEVICE_ID "{thingName}"
#endif'''.format(**config)

with open("./src/secret_aws.h", "w") as f:
  f.write(aws_secret)
  f.close()
print(" -- Successfully Write secret_aws.h -- ")

aws_topics = '''// THIS FILE IS AUTOGENERATED. DO NOT EDIT.

char aws_shadow_topic_get[]= "$aws/things/{clientId}/shadow/get";
char aws_shadow_topic_get_return[]= "$aws/things/{clientId}/shadow/get/accepted/raw";

char aws_shadow_topic_update[]= "$aws/things/{clientId}/shadow/update";
char aws_shadow_topic_update_return[]= "$aws/things/{clientId}/shadow/update/accepted/raw";

char aws_custom_ping[]= "hommieo/dev/{clientId}/ping";
'''.format(**config)

with open("./src/aws/awstopics.h", "w") as f:
  f.write(aws_topics)
  f.close()
print(" -- Successfully Write aws/awstopics.h -- ")

print("   - Input Sensors JSON      (unflattening): "+config.get('sensors'))
tempsensors = config.get('sensors').replace('"{', "{").replace('}"', "}")
tempsensors = tempsensors.replace('\\"','"').replace('"','\\"')
config['sensors'] = tempsensors
print("   - Output Sensors JSON        (flattened): "+config.get('sensors'))

print("   - Input Controllables JSON (unflattened): "+config.get('controllables'))
tempcontrollables = config.get('controllables').replace('"{', "{").replace('}"', "}")
tempcontrollables = tempcontrollables.replace('\\"','"').replace('"','\\"')
config['controllables'] = tempcontrollables
print("   - Output Controllables JSON  (flattened): "+config.get('controllables'))

product_config = '''// THIS FILE IS AUTOGENERATED. DO NOT EDIT.
char productId[] = "{productId}";
char verificationId[] = "{verificationId}";
int fwVersion = 1;

char sensor_init_json[] = "{sensors}";
char controlable_init_json[] = "{controllables}";
'''.format(**config)

with open ("./src/product_config.h", "w") as f:
  f.write(product_config)
  f.close()
print(" -- Successfully Write product_config.h -- ")

# delete output folder (and contents)
filesToDelete = glob.glob(options.output + "/*")
for f in filesToDelete:
  try:
    os.remove(f)
  except OSError as e:
    print("Error: %s - %s." % (e.filename, e.strerror))

print (" -- Successfully Delete Output Folder -- ")