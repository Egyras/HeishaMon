## HeatPumpType:

Assuming that bytes from #129 to #138 are unique for each model of Aquarea heat pump:

| Value |  Bytes 129 - 138 | IDU | ODU | KIT | Power [kW] | 1ph/3ph | T-CAP/HP |
| ---| --- | --- | --- | --- | --- | --- | ---|
| 0 | E2 CF 0B 13 33 32 D1 0C 16 33 | Monoblock | WH-MDC05H3E5 | Monoblock | 5 | 1ph | HP |
| 1 | E2 CF 0B 14 33 42 D1 0B 17 33 | Monoblock | WH-MDC07H3E5 | Monoblock | 7 | 1ph | HP |
| 2 | E2 CF 0D 77 09 12 D0 0B 05 11 | WH-SXC09H3E5 | WH-UX09HE5 | KIT-WXC09H3E5 | 9 | 1ph | T-CAP |
| 3 | E2 CF 0C 88 05 12 D0 0B 97 05 | WH-SDC09H3E8 | WH-UD09HE8 | KIT-WC09H3E8 | 9 | 3ph | HP |
| 4 | E2 CF 0D 85 05 12 D0 0C 94 05 | WH-SXC09H3E8 | WH-UX09HE8 | KIT-WXC09H9E8 | 9 | 3ph | T-CAP |
| 5 | E2 CF 0D 86 05 12 D0 0C 95 05 | WH-SXC12H9E8 | WH-UX12HE8 | KIT-WXC12H9E8 | 12 | 3ph | T-CAP |
| 6 | E2 CF 0D 87 05 12 D0 0C 96 05 | WH-SXC16H9E8 | WH-UX16HE8 | KIT-WXC16H9E8 | 16 | 3ph | T-CAP |
| 7 | E2 CE 0D 71 81 72 CE 0C 92 81 | WH-SDC05H3E5 | WH-UD05HE5 | KIT-WC05H3E5  | 5 | 1ph | HP |
| 8 | 62 D2 0B 43 54 42 D2 0B 72 66 | WH-SDC0709J3E5 | WH-UD09JE5 | KIT-WC09J3E5 | 9 | 1ph | HP |
| 9 | C2 D3 0B 33 65 B2 D3 0B 94 65 | Monoblock | WH-MDC05J3E5 | Monoblock | 5 | 1ph | HP |
|10 | E2 CF 0B 15 33 42 D1 0B 18 33 | Monoblock | WH-MDC09H3E5 | Monoblock | 9 | 1ph | HP |
|11 | E2 CF 0B 41 34 82 D1 0B 31 35 | Monoblock | WH-MXC09H3E5 | Monoblock | 9 | 1ph | T-CAP |
|12 | 62 D2 0B 45 54 42 D2 0B 47 55 | WH-ADC0309J3E5 | WH-UD09JE5 | KIT-ADC09JE5 | 9 | 1ph | HP - All-In-One | 
|13 | E2 CF 0C 74 09 12 D0 0D 95 05 | WH-ADC0916H9E8 | WH-UX12HE8 | KIT-AXC12HE8 | 12 | 3ph | T-CAP - All-In-One |
|14 | E2 CF 0B 82 05 12 D0 0C 91 05 | WH-SQC09H3E8 | WH-UQ09HE8 | KIT-WQC09H3E8 | 9 | 3ph | T-CAP - Super Quiet |
|15 | E2 CF 0C 55 14 12 D0 0B 15 08 | WH-SDC09H3E5 | WH-UD09HE5 | KIT-WC09H3E5 | 9 | 1 ph | HP |
|16 | E2 CF 0C 43 00 12 D0 0B 15 08 | WH-ADC0309H3E5 | WH-UD09HE5 | KIT-ADC09HE5 | 9 | 1 ph | HP - All-In-One |
|17 | 62 D2 0B 45 54 32 D2 0C 45 55 | WH-ADC0309J3E5 | WH-UD05JE5 | KIT-ADC05JE5 | 5 | 1ph | HP - All-In-One |
|18 | 62 D2 0B 43 54 42 D2 0C 46 55 | WH-SDC0709J3E5 | WH-UD07JE5 | KIT-WC07J3E5 | 7 | 1 ph | HP |
|19 | E2 CF 0C 54 14 12 D0 0B 14 08 | WH-SDC07H3E5-1 | WH-UD07HE5-1 | KIT-WC07H3E5-1 | 7 | 1 ph | HP |
|20 | C2 D3 0B 34 65 B2 D3 0B 95 65 | Monoblock | WH-MDC07J3E5 | Monoblock | 7 | 1ph | HP |
|21 | C2 D3 0B 35 65 B2 D3 0B 96 65 | Monoblock | WH-MDC09J3E5 | Monoblock | 9 | 1ph | HP |
|22 | 62 D2 0B 41 54 32 D2 0C 45 55 | WH-SDC0305J3E5 | WH-UD05JE5 | KIT-WC05J3E5 | 5 | 1ph | HP |
|23 | 32 D4 0B 87 84 73 90 0C 84 84 | Monoblock | WH-MXC09J3E8 | Monoblock | 9 | 3ph | T-CAP |
|24 | 32 D4 0B 88 84 73 90 0C 85 84 | MonoBlock | WH-MXC12J9E8 | Monoblock | 12 | 3ph | T-CAP |
|25 | E2 CF 0B 75 09 12 D0 0C 06 11 | WH-ADC1216H6E5 | WH-UD12HE5 | KIT-ADC12HE5 | 12 | 1ph | T-CAP |
|26 | 42 D4 0B 83 71 42 D2 0C 46 55 | WH-ADC0309J3E5C | WH-UD07JE5 | KIT-ADC07JE5C | 7 | 1ph | HP - All-In-One Compact |
|27 | C2 D3 0C 34 65 B2 D3 0B 95 65 | Monoblock | WH-MDC07J3E5 | Monoblock | 7 | 1ph | HP (new version?) |
|28 | C2 D3 0C 33 65 B2 D3 0B 94 65 | Monoblcok | WH-MDC05J3E5 | Monoblock | 5 | 1ph | HP (new version? |
|29 | E2 CF 0B 83 05 12 D0 0D 92 05 | WH-UQ12HE8 | WH-SQC12H9E8 | KIT-WQC12H9E8 | 12 | 3ph | T-CAP - Super Quiet |

All bytes are used for Heat Pump model identification in the code.

Note: These are the heat pump types that users have verified and reported back the model. HeishaMon should work with all H and J generation Aquarea heat pumps.
