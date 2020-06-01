## HeatPumpType:

Assuming that bytes from #129 to #138 are the unique for each type of Heat pump:

| Value |  Bytes 129 - 138 | IDU | ODU | KIT | Power [kW] | 1ph/3ph | T-CAP/HP |
| ---| --- | --- | --- | --- | --- | --- | ---|
| 0 | E2 CF 0B 13 33 32 D1 0C 16 33 | Monoblock | Mooblock | WH-MDC05H3E5 | 5 | 1ph | HP |
| 1 | E2 CF 0B 14 33 42 D1 0B 17 33 | Monoblock | Mooblock | WH-MDC07H3E5 | 7 | 1ph | HP |
| 2 | E2 CF 0D 77 09 12 D0 0B 05 11 | WH-SXC09H3E5 | WH-UX09HE5 | WXC09H3E5 | 9 | 1ph | T-CAP |
| 3 | E2 CF 0C 88 05 12 D0 0B 97 05 | WH-SDC09H3E8 | WH-UD09HE8 | WC09H3E8 | 9 | 3ph | HP |
| 4 | E2 CF 0D 86 05 12 D0 0C 95 05 | WH-SXC12H9E8 | WH-UX12HE8 | WXC12H9E8 | 12 | 3ph | T-CAP |
| 5 | E2 CF 0D 87 05 12 D0 0C 96 05 | WH-SXC16H9E8 | WH-UX16HE8 | WXC16H9E8 | 16 | 3ph | T-CAP |
| 6 | E2 CE 0D 71 81 72 CE 0C 92 81 | WH-SDC05H3E5 | WH-UD05HE5 | WC05H3E5  | 5 | 1ph | HP | 
| 7 | 62 D2 0B 43 54 42 D2 0B 72 66 | WH-SDC0709J3E5 | WH-UD09JE5 | WC09J3E5  | 9 | 1ph | HP | 

Byte 132 is taken for Heat Pump model.


