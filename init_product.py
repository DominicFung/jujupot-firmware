##
# PRE-REQ:
#   - jujupot-certs.zip file from Jujupot Admin site.
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
#  --input -i <input>    ::  default (../jujupot-certs.zip)
#  --output -o <output>  ::  default (./downloads)

import os
import json
import sys
import glob

from optparse import OptionParser
import zipfile

parser = OptionParser()
parser.add_option("-i", "--input", 
  dest="input", default="jujupot-certs.zip",
  help="Input file to read from. Default: jujupot-certs.zip"
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

char HOST_ADDRESS[]="{iotEndpoint}";
char CLIENT_ID[]="{clientId}";
char deviceId[]="{thingName}";'''.format(
  iotEndpoint=config['iotEndpoint'], 
  clientId=config['clientId'],
  thingName=config['thingName']
)

with open("./src/secret_aws.h", "w") as f:
  f.write(aws_secret)
  f.close()

print(" -- Successfully Write secret_aws.h -- ")

# delete output folder (and contents)
filesToDelete = glob.glob(options.output + "/*")
for f in filesToDelete:
  try:
    os.remove(f)
  except OSError as e:
    print("Error: %s - %s." % (e.filename, e.strerror))

print (" -- Successfully Delete Output Folder -- ")