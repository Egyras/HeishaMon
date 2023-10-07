## Protocol byte decrypt info for the new data block on newer models:

|  Topic# | Byte# | Possible Value | Value decrypt | Value Description |
| :---- | ---- | ---- | ----- | ----:|
| TOP | 0 | 71 | | Header|
| TOP | 1 | C8 | Data length ( Packet length = Data length + 3 )| Header|
| TOP | 2 | 01 | | Header|
| TOP | 3 | 21 | This is 0x21 for new data block | Header|
| TOP | 4 | 8A | | |
| TOP | 5 | EA | | |
| TOP | 6 | 01 | | |
| TOP | 7 | 00 | | |
| TOP | 8 | 00 | | |
| TOP | 9 | 00 | | |
| TOP | 10 | 00 | | |
| TOP | 11 | 00 | | |
| TOP | 12 | 00 | | |
| TOP | 13 | 00 | | |
| TOP | 14 | 30 | Little-endian uint_16 with next byte | Power usage for heat in Watt |
| TOP | 15 | 03 | Little-endian uint_16 with previous byte | Power usage for heat in Watt |
| TOP | 16 | 01 | | |
| TOP | 17 | 00 | | |
| TOP | 18 | 01 | | |
| TOP | 19 | 00 | | |
| TOP | 20 | 48 | Little-endian uint_16 with next byte | Power generated for heat in Watt  |
| TOP | 21 | 07 | Little-endian uint_16 with previous byte | Power generated for heat in Watt |
| TOP | 22 | 01 | | |
| TOP | 23 | 00 | | |
| TOP | 24 | 01 | | |
| TOP | 25 | 00 | | |
| TOP | 26 | 01 | | |
| TOP | 27 | 00 | | |
| TOP | 28 | 00 | | |
| TOP | 29 | 00 | | |
| TOP | 30 | 01 | | |
| TOP | 31 | 00 | | |
| TOP | 32 | 00 | | |
| TOP | 33 | 00 | | |
| TOP | 34 | 01 | | |
| TOP | 35 | 00 | | |
| TOP | 36 | 00 | | |
| TOP | 37 | 00 | | |
| TOP | 38 | 01 | | |
| TOP | 39 | 00 | | |
| TOP | 40 | 00 | | |
| TOP | 41 | 00 | | |
| TOP | 42 | 01 | | |
| TOP | 43 | 00 | | |
| TOP | 44 | 01 | | |
| TOP | 45 | 00 | | |
| TOP | 46 | 01 | | |
| TOP | 47 | 00 | | |
| TOP | 48 | 00 | | |
| TOP | 49 | 00 | | |
| TOP | 50 | 00 | | |
| TOP | 51 | 00 | | |
| TOP | 52 | 00 | | |
| TOP | 53 | 00 | | |
| TOP | 54 | 00 | | |
| TOP | 55 | 00 | | |
| TOP | 56 | 00 | | |
| TOP | 57 | 00 | | |
| TOP | 58 | 00 | | |
| TOP | 59 | 00 | | |
| TOP | 60 | 00 | | |
| TOP | 61 | 00 | | |
| TOP | 62 | 00 | | |
| TOP | 63 | 00 | | |
| TOP | 64 | 00 | | |
| TOP | 65 | 00 | | |
| TOP | 66 | 00 | | |
| TOP | 67 | 00 | | |
| TOP | 68 | 00 | | |
| TOP | 69 | 00 | | |
| TOP | 70 | 00 | | |
| TOP | 71 | 00 | | |
| TOP | 72 | 00 | | |
| TOP | 73 | 00 | | |
| TOP | 74 | 00 | | |
| TOP | 75 | 00 | | |
| TOP | 76 | 00 | | |
| TOP | 77 | 00 | | |
| TOP | 78 | 00 | | |
| TOP | 79 | 00 | | |
| TOP | 80 | 00 | | |
| TOP | 81 | 00 | | |
| TOP | 82 | 00 | | |
| TOP | 83 | 00 | | |
| TOP | 84 | 00 | | |
| TOP | 85 | 00 | | |
| TOP | 86 | 00 | | |
| TOP | 87 | 00 | | |
| TOP | 88 | 00 | | |
| TOP | 89 | 00 | | |
| TOP | 90 | 00 | | |
| TOP | 91 | 00 | | |
| TOP | 92 | 00 | | |
| TOP | 93 | 00 | | |
| TOP | 94 | 00 | | |
| TOP | 95 | 00 | | |
| TOP | 96 | 00 | | |
| TOP | 97 | 00 | | |
| TOP | 98 | 00 | | |
| TOP | 99 | 00 | | |
| TOP | 100 | 00 | | |
| TOP | 101 | 00 | | |
| TOP | 102 | 00 | | |
| TOP | 103 | 00 | | |
| TOP | 104 | 00 | | |
| TOP | 105 | 00 | | |
| TOP | 106 | 00 | | |
| TOP | 107 | 00 | | |
| TOP | 108 | 00 | | |
| TOP | 109 | 00 | | |
| TOP | 110 | 00 | | |
| TOP | 111 | 00 | | |
| TOP | 112 | 00 | | |
| TOP | 113 | 00 | | |
| TOP | 114 | 00 | | |
| TOP | 115 | 00 | | |
| TOP | 116 | 00 | | |
| TOP | 117 | 00 | | |
| TOP | 118 | 00 | | |
| TOP | 119 | 00 | | |
| TOP | 120 | 00 | | |
| TOP | 121 | 00 | | |
| TOP | 122 | 00 | | |
| TOP | 123 | 00 | | |
| TOP | 124 | 00 | | |
| TOP | 125 | 00 | | |
| TOP | 126 | 00 | | |
| TOP | 127 | 00 | | |
| TOP | 128 | 00 | | |
| TOP | 129 | 00 | | |
| TOP | 130 | 00 | | |
| TOP | 131 | 00 | | |
| TOP | 132 | 00 | | |
| TOP | 133 | 00 | | |
| TOP | 134 | 00 | | |
| TOP | 135 | 00 | | |
| TOP | 136 | 00 | | |
| TOP | 137 | 00 | | |
| TOP | 138 | 00 | | |
| TOP | 139 | 00 | | |
| TOP | 140 | 00 | | |
| TOP | 141 | 00 | | |
| TOP | 142 | 00 | | |
| TOP | 143 | 00 | | |
| TOP | 144 | 00 | | |
| TOP | 145 | 00 | | |
| TOP | 146 | 00 | | |
| TOP | 147 | 00 | | |
| TOP | 148 | 00 | | |
| TOP | 149 | 00 | | |
| TOP | 150 | 00 | | |
| TOP | 151 | 00 | | |
| TOP | 152 | 00 | | |
| TOP | 153 | 00 | | |
| TOP | 154 | 00 | | |
| TOP | 155 | 00 | | |
| TOP | 156 | 00 | | |
| TOP | 157 | 00 | | |
| TOP | 158 | 00 | | |
| TOP | 159 | 00 | | |
| TOP | 160 | 00 | | |
| TOP | 161 | 00 | | |
| TOP | 162 | 00 | | |
| TOP | 163 | 00 | | |
| TOP | 164 | 00 | | |
| TOP | 165 | 00 | | |
| TOP | 166 | 00 | | |
| TOP | 167 | 00 | | |
| TOP | 168 | 00 | | |
| TOP | 169 | 00 | | |
| TOP | 170 | 00 | | |
| TOP | 171 | 00 | | |
| TOP | 172 | 00 | | |
| TOP | 173 | 00 | | |
| TOP | 174 | 00 | | |
| TOP | 175 | 00 | | |
| TOP | 176 | 00 | | |
| TOP | 177 | 00 | | |
| TOP | 178 | 00 | | |
| TOP | 179 | 00 | | |
| TOP | 180 | 00 | | |
| TOP | 181 | 00 | | |
| TOP | 182 | 00 | | |
| TOP | 183 | 00 | | |
| TOP | 184 | 00 | | |
| TOP | 185 | 00 | | |
| TOP | 186 | 00 | | |
| TOP | 187 | 00 | | |
| TOP | 188 | 00 | | |
| TOP | 189 | 00 | | |
| TOP | 190 | 00 | | |
| TOP | 191 | 00 | | |
| TOP | 192 | 00 | | |
| TOP | 193 | 00 | | |
| TOP | 194 | 00 | | |
| TOP | 195 | 00 | | |
| TOP | 196 | 00 | | |
| TOP | 197 | 00 | | |
| TOP | 198 | 00 | | |
| TOP | 199 | 00 | | |
| TOP | 200 | 00 | | |
| TOP | 201 | 00 | | |
| TOP | 202 | A3 | | |




To get decimal values you must convert from hexadecimal and do some calulation depending on value. 
Panasonic query, answer and commands are using 8-bit Checksum to verify serial data ( sum(all bytes) & 0xFF == 0 ). Last byte is checksum value.


## Query Examples:

Panasonic query:

`71 6C 01 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 12`

Panasonic answer example:

`71 C8 01 21 8A EA 01 00 00 00 00 00 00 00 30 03 01 00 01 00 48 07 01 00 01 00 01 00 00 00 01 00 
00 00 01 00 00 00 01 00 00 00 01 00 01 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 A3 `
