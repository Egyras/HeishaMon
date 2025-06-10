#!/usr/bin/env bash
cd HeishaMon
arduino-cli compile --output-dir . \
  --fqbn=esp32:esp32:esp32s3:CDCOnBoot=cdc,PSRAM=enabled,PartitionScheme=min_spiffs \
  --warnings=none --verbose HeishaMon.ino
