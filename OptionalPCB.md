## Optional PCB:

Optional PCB is also connected to HP via the CN-CNT ( in case using it and CZ-TAW1 ,Optional PCB is conected to HP , and on Optional PCB there is another CN-CNT to connect CZ-TAW1).
In this way Optional PCB comunicate with HP in similar way to CZ-TAW1 - Next to(between) standard Magic Packets there specific commands from Optional PCB to HP ,and next to (beetwen) answers from HP there is a answer/confirmation to Optional PCB.


### Set command byte decrypt:

| Byte# | Possible Value | Value decrypt | Value Description |
|:---- | ---- | ----- | ----:|
| 00 | F1 |   | Header  |
| 01 | 11 | Data length ( Packet length = Data length + 3 ) |  Header |
| 02 | 01 |   | Header  |
| 03 | 50 |   | Header  |
| 04 | 00 |   | 0 byte  |
| 05 | 00 |   | 0 byte  |
| 06 | 40 | HEX:  40 - SG Mode 0,0 (Normal) , 60 - SG Mode 1,0 ( HP and DHW off) ,50 - SG Mode 0,1(Capacity 1) ,70 - SG Mode 1,1(Capacity 2)  ,00 - Compressor external SW on  | SG ready values , External Compressor SW  |
| 07 | FF |  NTC 6,5kOhm resistance characteristic value | Temp. Pool  |
| 08 | FF |  NTC 6,5kOhm resistance characteristic value | Temp. Buffer  |
| 09 | E5 |   | ?  |
| 10 | FF |  NTC 6,5kOhm resistance characteristic value | Possible Temp. Z2_Room   |
| 11 | FF |  NTC 6,5kOhm resistance characteristic value | Possible Temp. Z1_Room   |
| 12 | 00 |   | 0 byte  |
| 13 | FF |  NTC 6,5kOhm resistance characteristic value | Temp. Solar  |
| 14 | EA | HEX:  EB-100% ,B8 - 75% ,85 -50%,52 - 25% ,2B - 5% (proportional values should works) | Demand Control  |
| 15 | FF |  NTC 6,5kOhm resistance characteristic value | Temp. Z2_Water   |
| 16 | FF |  NTC 6,5kOhm resistance characteristic value | Temp. Z1_Water   |
| 17 | 00 |   | 0 byte  |
| 18 | 00 |   | 0 byte  |
| 19 | 2C |  CHECKSUM |  |

## NTC 6,5kOhm characteristic:

Values are direct measurement of NTC thermistor conected to Optional PCB.
It can be aproximate by function : ?????

#### Exact values table 


| HEX Val. | Temp [C] | HEX Val. | Temp [C] | Hex Val. | Temp [C] | Hex Val. | Temp [C] |
|--- | --- | --- | --- | --- | --- | ---- | --- |
| 00 | 120 | 40 | 53 | 80 |  | C0 |  |
| 01 | 120 | 41 |  | 81 |  | C1 |  |
| 02 | 120 | 42 |  | 82 |  | C2 |  |
| 03 | 120 | 43 |  | 83 |  | C3 |  |
| 04 | 120 | 44 |  | 84 |  | C4 |  |
| 05 | 120 | 45 |  | 85 |  | C5 |  |
| 06 | 120 | 46 |  | 86 |  | C6 |  |
| 07 | 120 | 47 |  | 87 |  | C7 |  |
| 08 | 120 | 48 |  | 88 |  | C8 |  |
| 09 | 120 | 49 |  | 89 |  | C9 |  |
| 0A | 120 | 4A |  | 8A |  | CA |  |
| 0B | 120 | 4B |  | 8B |  | CB |  |
| 0C |  | 4C |  | 8C |  | CC |  |
| 0D | 114 | 4D |  | 8D |  | CD |  |
| 0E |  | 4E |  | 8E |  | CE |  |
| 0F |  | 4F |  | 8F |  | CF |  |
| 10 |  | 50 |  | 90 |  | D0 |  |
| 11 |  | 51 |  | 91 |  | D1 |  |
| 12 |  | 52 |  | 92 |  | D2 |  |
| 13 |  | 53 |  | 93 |  | D3 |  |
| 14 |  | 54 |  | 94 |  | D4 |  |
| 15 |  | 55 |  | 95 |  | D5 |  |
| 16 |  | 56 |  | 96 |  | D6 |  |
| 17 |  | 57 |  | 97 |  | D7 |  |
| 18 |  | 58 |  | 98 |  | D8 |  |
| 19 |  | 59 |  | 99 |  | D9 |  |
| 1A |  | 5A |  | 9A |  | DA |  |
| 1B |  | 5B |  | 9B |  | DB |  |
| 1C |  | 5C |  | 9C |  | DC |  |
| 1D |  | 5D |  | 9D |  | DD |  |
| 1E |  | 5E |  | 9E |  | DE |  |
| 1F |  | 5F |  | 9F |  | DF |  |
| 20 |  | 60 |  | A0 |  | E0 |  |
| 21 |  | 61 |  | A1 |  | E1 |  |
| 22 |  | 62 |  | A2 |  | E2 |  |
| 23 |  | 63 |  | A3 |  | E3 |  |
| 24 |  | 64 |  | A4 |  | E4 |  |
| 25 |  | 65 |  | A5 |  | E5 | -20  |
| 26 |  | 66 |  | A6 |  | E6 |  |
| 27 |  | 67 |  | A7 |  | E7 |  |
| 28 |  | 68 |  | A8 |  | E8 |  |
| 29 |  | 69 |  | A9 |  | E9 |  |
| 2A |  | 6A |  | AA |  | EA | -24 |
| 2B |  | 6B |  | AB |  | EB |  |
| 2C |  | 6C |  | AC |  | EC |  |
| 2D |  | 6D |  | AD |  | ED |  |
| 2E |  | 6E |  | AE |  | EE |  |
| 2F |  | 6F |  | AF |  | EF |  |
| 30 |  | 70 |  | B0 |  | E0 |  |
| 31 |  | 71 |  | B1 |  | F1 |  |
| 32 |  | 72 |  | B2 |  | F2 |  |
| 33 |  | 73 | 29 | B3 |  | F3 |  |
| 34 |  | 74 |  | B4 |  | F4 |  |
| 35 |  | 75 |  | B5 |  | F5 |  |
| 36 |  | 76 |  | B6 |  | F6 |  |
| 37 |  | 77 |  | B7 |  | F7 |  |
| 38 |  | 78 |  | B8 |  | F8 |  |
| 39 |  | 79 | 27 | B9 |  | F9 |  |
| 3A |  | 7A |  | BA |  | FA |  |
| 3B |  | 7B |  | BB |  | FB |  |
| 3C |  | 7C |  | BC |  | FC |  |
| 3D |  | 7D |  | BD |  | FD |  |
| 3E |  | 7E |  | BE |  | FE |  |
| 3F |  | 7F |  | BF |  | FF | -78 |


### Answer/confirmation from HP to Optional PCB is always the same:

`71 11 01 50 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 2D`

### To do:

- Heat/Cool Switch 

- Optional Thermostat 1 &2 ( Cool / Heat)
