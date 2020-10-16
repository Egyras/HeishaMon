## Optional PCB:

Optional PCB is also connected to HP via the CN-CNT ( in case using it and CZ-TAW1 ,Optional PCB is conected to HP , and on Optional PCB there is another CN-CNT to connect CZ-TAW1).
In this way Optional PCB comunicate with HP in similar way to CZ-TAW1 - Next to(between) standard Magic Packets there specific commands from Optional PCB to HP ,and next to (beetwen) answers from HP there is a answer/confirmation to Optional PCB.

### Optional PCB emulation support:
Recent firmware allows (experimental) support for optional PCB emulation. This allows you to set SmartGrid or Demand Control values without having the optional pcb installed. Also you can send the temperatures normally connected to that board to the heatpump.

You can publish mqtt messages towards the 'topic base/pcb/pcb topic', so for example "panasonic_heat_pump/pcb/Solar_Temp". For temperatures you just send the real temperature (the hexadecimal value will be calculated for your). For SmartGrid and Demand control you send the decimal representation of the hex value you want to send (see below for the possible hex values).

### Set command byte decrypt:

| PCB Topic | Byte# | Possible Value | Value decrypt | Value Description |
|:---- | ---- | ---- | ----- | ----:|
| | 00 | F1 |   | Header  |
| | 01 | 11 | Data length ( Packet length = Data length + 3 ) |  Header |
| | 02 | 01 |   | Header  |
| | 03 | 50 |   | Header  |
| | 04 | 00 |   | 0 byte  |
| | 05 | 00 |   | 0 byte  |
| Heat_Cool_Mode<br/>Compressor_State<br/>SmartGrid_Mode<br/>External_Thermostat_1_State<br/>External_Thermostat_2_State | 06 | 40 | 1st bit = Heat/Cool<br/>2nd bit = Compressor state<br/>3rd/4th bit == SmartGrid Mode (00 = normal, 10 = HP/DHW off, 01 = Capacity 1, 11 = Capacity 2)<br/>5th/6th bit = Thermostat 1 (00 = no demand, 10 = heat demand, 01 = cool demand, 11 = heat and cool demand)<br/>7th/8th bit = Thermostat 2 (00 = no demand, 10 = heat demand, 01 = cool demand, 11 = heat and cool demand)  | SG ready values , External Compressor SW , Heat/Cool SW, Thermostat 1/2 |
| Pool_Temp | 07 | FF |  NTC 6,5kOhm resistance characteristic value | Temp. Pool  |
| Buffer_Temp | 08 | FF |  NTC 6,5kOhm resistance characteristic value | Temp. Buffer  |
| | 09 | E5 |   | ?  |
| Z1_Room_Temp | 10 | FF |  NTC 6,5kOhm resistance characteristic value | Temp. Z1_Room   |
| Z2_Room_Temp | 11 | FF |  NTC 6,5kOhm resistance characteristic value | Temp. Z2_Room   |
| | 12 | 00 |   | 0 byte  |
| Solar_Temp | 13 | FF |  NTC 6,5kOhm resistance characteristic value | Temp. Solar  |
| Demand_Control | 14 | EA | HEX:  EB-100% ,B8 - 75% ,85 -50%,52 - 25% ,2B - 5% (proportional values should works) | Demand Control  |
| Z2_Water_Temp | 15 | FF |  NTC 6,5kOhm resistance characteristic value | Temp. Z2_Water   |
| Z1_Water_Temp | 16 | FF |  NTC 6,5kOhm resistance characteristic value | Temp. Z1_Water   |
| | 17 | 00 |   | 0 byte  |
| | 18 | 00 |   | 0 byte  |
| | 19 | 2C |  CHECKSUM |  |

## NTC 6,5kOhm characteristic:

Values are direct measurement of NTC thermistor conected to Optional PCB.
It can be aproximate by function : Uref * (RT / (Rf + RT)) where Uref = 255 and Rf=6480. RT is calculated as R25 * exp(constant * (1 / (temp + K) - 1 / (T25 + K))) where R25 = 6340, T25=25, K=273.15, constant=3695 and temp the input temperature.

#### Exact values table 


| HEX Val. | Temp [C] | HEX Val. | Temp [C] | Hex Val. | Temp [C] | Hex Val. | Temp [C] |
|--- | --- | --- | --- | --- | --- | ---- | --- |
| 00 | 120 | 40 | 53 | 80 | 24 | C0 | 0 |
| 01 | 120 | 41 | 53 | 81 | 24 | C1 | 0 |
| 02 | 120 | 42 | 52 | 82 | 24 | C2 | -1 |
| 03 | 120 | 43 | 51 | 83 | 23 | C3 | -1 |
| 04 | 120 | 44 | 51 | 84 | 23 | C4 | -2 |
| 05 | 120 | 45 | 50 | 85 | 22 | C5 | -2 |
| 06 | 120 | 46 | 50 | 86 | 22 | C6 | -3 |
| 07 | 120 | 47 | 49 | 87 | 22 | C7 | -3 |
| 08 | 120 | 48 | 49 | 88 | 21 | C8 | -4 |
| 09 | 120 | 49 | 48 | 89 | 21 | C9 | -4 |
| 0A | 120 | 4A | 48 | 8A | 21 | CA | -4 |
| 0B | 120 | 4B | 47 | 8B | 20 | CB | -5 |
| 0C | 117 | 4C | 47 | 8C | 20 | CC | -5 |
| 0D | 114 | 4D | 46 | 8D | 19 | CD | -6 |
| 0E | 111 | 4E | 45 | 8E | 19 | CE | -6 |
| 0F | 108 | 4F | 45 | 8F | 19 | CF | -7 |
| 10 | 106 | 50 | 44 | 90 | 18 | D0 | -7 |
| 11 | 103 | 51 | 44 | 91 | 18 | D1 | -8 |
| 12 | 101 | 52 | 44 | 92 | 18 | D2 | -8 |
| 13 | 99 | 53 | 43 | 93 | 17 | D3 | -9 |
| 14 | 97 | 54 | 43 | 94 | 17 | D4 | -9 |
| 15 | 95 | 55 | 42 | 95 | 17 | D5 | -10 |
| 16 | 93 | 56 | 42 | 96 | 16 | D6 | -10 |
| 17 | 92 | 57 | 41 | 97 | 16 | D7 | -11 |
| 18 | 90 | 58 | 41 | 98 | 15 | D8 | -12 |
| 19 | 88 | 59 | 40 | 99 | 15 | D9 | -12 |
| 1A | 87 | 5A | 40 | 9A | 15 | DA | -13 |
| 1B | 86 | 5B | 39 | 9B | 14 | DB | -13 |
| 1C | 84 | 5C | 39 | 9C | 14 | DC | -14 |
| 1D | 83 | 5D | 38 | 9D | 14 | DD | -15 |
| 1E | 82 | 5E | 38 | 9E | 13 | DE | -15 |
| 1F | 80 | 5F | 38 | 9F | 13 | DF | -16 |
| 20 | 79 | 60 | 37 | A0 | 12 | E0 | -16 |
| 21 | 78 | 61 | 37 | A1 | 12 | E1 | -17 |
| 22 | 77 | 62 | 36 | A2 | 12 | E2 | -18 |
| 23 | 76 | 63 | 36 | A3 | 11 | E3 | -18 |
| 24 | 75 | 64 | 35 | A4 | 11 | E4 | -19 |
| 25 | 74 | 65 | 35 | A5 | 11 | E5 | -20  |
| 26 | 73 | 66 | 35 | A6 | 10 | E6 | -21 |
| 27 | 72 | 67 | 34 | A7 | 10 | E7 | -21 |
| 28 | 71 | 68 | 34 | A8 | 9 | E8 | -22 |
| 29 | 70 | 69 | 33 | A9 | 9 | E9 | -23 |
| 2A | 69 | 6A | 33 | AA | 9 | EA | -24 |
| 2B | 68 | 6B | 32 | AB | 8 | EB | -25 |
| 2C | 67 | 6C | 32 | AC | 8 | EC | -26 |
| 2D | 66 | 6D | 32 | AD | 8 | ED | -27 |
| 2E | 66 | 6E | 31 | AE | 7 | EE | -28 |
| 2F | 65 | 6F | 31 | AF | 7 | EF | -29 |
| 30 | 64 | 70 | 30 | B0 | 6 | F0 | -30 |
| 31 | 63 | 71 | 30 | B1 | 6 | F1 | -31 |
| 32 | 62 | 72 | 30 | B2 | 6 | F2 | -32 |
| 33 | 62 | 73 | 29 | B3 | 5 | F3 | -33 |
| 34 | 61 | 74 | 29 | B4 | 5 | F4 | -35 |
| 35 | 60 | 75 | 28 | B5 | 4 | F5 | -36 |
| 36 | 60 | 76 | 28 | B6 | 4 | F6 | -38 |
| 37 | 59 | 77 | 28 | B7 | 4 | F7 | -40 |
| 38 | 58 | 78 | 27 | B8 | 3 | F8 | -41 |
| 39 | 58 | 79 | 27 | B9 | 3 | F9 | -44 |
| 3A | 57 | 7A | 27 | BA | 2 | FA | -46 |
| 3B | 56 | 7B | 26 | BB | 2 | FB | -49 |
| 3C | 56 | 7C | 26 | BC | 2 | FC | -53 |
| 3D | 55 | 7D | 25 | BD | 1 | FD | -57 |
| 3E | 54 | 7E | 25 | BE | 1 | FE | -64 |
| 3F | 54 | 7F | 25 | BF | 0 | FF | -78 |

### Answer/confirmation from HP to Optional PCB is always the same:

`71 11 01 50 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 2D`

### To do:

- Optional Thermostat 1 &2 ( Cool / Heat)
- Issues with Zones ( mixing valves , pump flows )
