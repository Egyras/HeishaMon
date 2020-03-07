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
| 00 | 120 | 40 | 53 | 80 | 24 | C0 |  |
| 01 | 120 | 41 |  | 81 |  | C1 |  |
| 02 | 120 | 42 | 52 | 82 |  | C2 | -1 |
| 03 | 120 | 43 | 51 | 83 | 23 | C3 |  |
| 04 | 120 | 44 |  | 84 |  | C4 | -2 |
| 05 | 120 | 45 | 46 | 85 | 22 | C5 |  |
| 06 | 120 | 46 |  | 86 |  | C6 | -3 |
| 07 | 120 | 47 | 49 | 87 |  | C7 |  |
| 08 | 120 | 48 |  | 88 | 21 | C8 | -4 |
| 09 | 120 | 49 |  | 89 |  | C9 |  |
| 0A | 120 | 4A | 48 | 8A | 8B | CA |  |
| 0B | 120 | 4B | 47 | 8B |  | CB | -5 |
| 0C | 117 | 4C |  | 8C |  | CC |  |
| 0D | 114 | 4D | 46 | 8D | 19 | CD | -6 |
| 0E | 111 | 4E | 45 | 8E |  | CE |  |
| 0F | 108 | 4F |  | 8F |  | CF | -7 |
| 10 | 106 | 50 | 44 | 90 | 18 | D0 |  |
| 11 | 103 | 51 |  | 91 |  | D1 | -8 |
| 12 | 101 | 52 |  | 92 |  | D2 |  |
| 13 | 99 | 53 | 43 | 93 | 17 | D3 | -9 |
| 14 | 97 | 54 |  | 94 |  | D4 |  |
| 15 | 95 | 55 | 42 | 95 |  | D5 | -10 |
| 16 | 93 | 56 |  | 96 | 16 | D6 |  |
| 17 | 92 | 57 | 41 | 97 |  | D7 | -11 |
| 18 | 90 | 58 |  | 98 | 15 | D8 |  |
| 19 | 88 | 59 | 40 | 99 |  | D9 | -12 |
| 1A | 87 | 5A |  | 9A |  | DA | -13 |
| 1B | 86 | 5B | 39 | 9B | 14 | DB |  |
| 1C | 84 | 5C |  | 9C |  | DC | -14 |
| 1D | 83 | 5D | 38 | 9D |  | DD |  |
| 1E | 82 | 5E |  | 9E | 13 | DE | -15 |
| 1F | 80 | 5F |  | 9F |  | DF | -16 |
| 20 | 79 | 60 | 37 | A0 |  | E0 |  |
| 21 | 78 | 61 |  | A1 | 12 | E1 | -17 |
| 22 | 77 | 62 | 36 | A2 |  | E2 | -18 |
| 23 | 76 | 63 |  | A3 |  | E3 |  |
| 24 | 75 | 64 | 35 | A4 | 11 | E4 | -19 |
| 25 | 74 | 65 |  | A5 |  | E5 | -20 |
| 26 | 73 | 66 |  | A6 |  | E6 | -21 |
| 27 | 72 | 67 | 34 | A7 |  | E7 |  |
| 28 | 71 | 68 |  | A8 | 10 | E8 | -22 |
| 29 | 70 | 69 | 33 | A9 | 9 | E9 | -23 |
| 2A | 69 | 6A |  | AA |  | EA | -24 |
| 2B | 68 | 6B | 32 | AB | 8 | EB | -25 |
| 2C | 67 | 6C |  | AC |  | EC | -26 |
| 2D | 66 | 6D |  | AD |  | ED | -27 |
| 2E |  | 6E | 31 | AE | 7 | EE | -28 |
| 2F | 65 | 6F |  | AF |  | EF | -29 |
| 30 | 64 | 70 | 30 | B0 | 6 | F0 | -30 |
| 31 | 63 | 71 |  | B1 |  | F1 | -31 |
| 32 | 62 | 72 |  | B2 |  | F2 | -32 |
| 33 |  | 73 | 29 | B3 | 5 | F3 | -33 |
| 34 | 61 | 74 |  | B4 |  | F4 | -35 |
| 35 | 60 | 75 | 28 | B5 | 4 | F5 | -36 |
| 36 |  | 76 |  | B6 |  | F6 | -38 |
| 37 | 59 | 77 |  | B7 |  | F7 | -40 |
| 38 | 58 | 78 | 27 | B8 | 3 | F8 | -41 |
| 39 |  | 79 |  | B9 |  | F9 | -44 |
| 3A | 57 | 7A |  | BA | 2 | FA | -46 |
| 3B | 56 | 7B | 26 | BB |  | FB | -49 |
| 3C |  | 7C |  | BC | 2 | FC | -53 |
| 3D | 55 | 7D | 25 | BD | 1 | FD | -57 |
| 3E | 54 | 7E |  | BE |  | FE | -64 |
| 3F |  | 7F |  | BF | 0 | FF | -78 |



### Answer/confirmation from HP to Optional PCB is always the same:

`71 11 01 50 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 2D`

### To do:

- Heat/Cool Switch 

- Optional Thermostat 1 &2 ( Cool / Heat)
