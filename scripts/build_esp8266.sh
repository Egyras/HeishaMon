#!/usr/bin/env bash
cd HeishaMon
arduino-cli compile --output-dir . \
  --fqbn=esp8266:esp8266:d1_mini:xtal=160,vt=flash,ssl=basic,mmu=3216,non32xfer=fast,eesz=4M2M,ip=lm2f,dbg=Disabled,lvl=None____,wipe=none,baud=921600 \
  --warnings=none --verbose HeishaMon.ino
