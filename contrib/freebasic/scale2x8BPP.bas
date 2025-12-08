'Compiler FreeBASIC
'Scale2x algo (used im MAME, VisualBoy Advance, etc)
'Relsoft 2005
'Rel.Betterwebber.com                  
'Special thanks to Andrea Mazzoleni for the Original ALGO.
'http://scale2x.sourceforge.net/history.html

defint a-z
option explicit

const TILE_WID =  48
const TILE_HEI =  48
const SCR_WIDTH = 320
const SCR_HEIGHT = 240

DECLARE SUB InitImageData (FileName$, ImageArray() as Ushort)
DECLARE SUB MakeImageIndex (ImageArray() as Ushort, IndexArray() as Ushort)
DECLARE SUB LoadPalPp256 (File$)



SCREEN 14,8,,1


Dim Image(TILE_WID - 1,TILE_HEI -1 ) as Ubyte
Dim Image2x(TILE_WID*2 -1 ,TILE_HEI*2 -1) as Ubyte
Redim shared Sprite(1) as Ushort
Redim shared SpriteIndex(1) as Ushort





dim i as integer, j as integer
dim x as integer, y as integer
dim x2 as integer, y2 as integer
dim red as integer, grn as integer, blu as integer
dim col as Ubyte

'Scale2x stuff
dim E as Ubyte, E0 as Ubyte, E1 as Ubyte, E2 as Ubyte, E3 as Ubyte
dim B as Ubyte, D as Ubyte, F as Ubyte, H as Ubyte


InitImageData "", Sprite()
MakeImageIndex Sprite(), SpriteIndex()
LoadPalPp256 ""

Put(0,0), Sprite(1)



for y = 0 to TILE_HEI - 1
    for x = 0 to TILE_WID - 1
        col = point(x,y)
        Image(x, y) = col
    next x
next y

for y = 0 to (TILE_HEI - 1)
    for x = 0 to (TILE_WID - 1)
        x2 = x * 2
        y2 = y * 2
        for i = 0 to 1
        for j = 0 to 1
            Image2x(x2+i,y2+j) = Image(x,y)
        next j
        next i
    next x
next y


for y = 0 to (TILE_HEI ) * 2 - 1            'normal blocky scale
    for x = 0 to (TILE_WID ) * 2 - 1
        Pset(x,100+y), Image2x(x,y)
    next x
next y


for y = 0 to (TILE_HEI - 1)                 'scale2x
    for x = 0 to (TILE_WID - 1)
        x2 = (x * 2) + 150
        y2 = (y * 2) + 100
        B= Image(x,y - 1)
        D= Image(x - 1,y)
        E= Image(x,y)
        F= Image(x + 1,y)
        H= Image(x,y + 1)
        if B <> H and D <> F then
            if D = B then E0 = D else E0 = E
            if B = F then E1 = F else E1 = E
            if D = H then E2 = D else E2 = E
            if H = F then E3 = F else E3 = E
        else
        	E0 = E
        	E1 = E
        	E2 = E
        	E3 = E
        end if

        Pset(x2,y2), E0
        Pset(x2 + 1,y2), E1
        Pset(x2,y2 + 1), E2
        Pset(x2 + 1,y2 + 1), E3

    next x
next y

Locate 3, 8
Print " <- Original Image"

Locate 12, 0
Print "Normal Scale"

Locate 12, 20
Print "Scale2x"





SLEEP

end


ImageData: 'ZERO.PUT image data.
DATA 1154
DATA 384,48,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
DATA 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
DATA 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
DATA 0,0,4608,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
DATA 0,0,0,0,4608,0,4608,18,0,0,0,0,0,0,0,0,0,0,0,0
DATA 0,0,0,0,0,0,0,0,4608,18,4608,4850,0,0,0,0,0,0,0,0
DATA 0,0,0,0,0,0,0,0,0,0,0,0,4608,4626,4608,-31186,18,0,0,0
DATA 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4626,4626,-3566,-3538
DATA 4742,4608,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
DATA 5906,7710,-3450,11822,4850,4626,0,0,0,0,0,0,0,0,0,0,0,0,0,0
DATA 0,0,0,4608,7707,6942,11910,11822,4850,4850,0,0,0,0,0,0,0,0,0,0
DATA 0,0,0,0,0,0,0,5906,7710,-31202,12018,11822,-3342,4850,0,0,0,0,0,0
DATA 0,0,0,0,0,0,0,0,0,0,0,2334,7849,-31209,-31186,11822,-3342,4742,0,0
DATA 0,0,0,0,0,0,0,0,0,0,0,0,0,0,4608,2313,7849,11910,4742,-3538
DATA -30990,4742,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4608,-22263
DATA -31202,-31186,4626,-3538,-31098,4626,0,4626,18,0,0,0,0,0,0,0,0,0,0,0
DATA 0,0,4608,7849,11910,4742,4742,6958,4638,-4334,4626,-3538,18,0,0,0,0,0,0,0
DATA 0,0,0,0,0,0,4608,-31205,-31186,7698,4792,7726,6930,4626,5911,-31214,18,0,0,0
DATA 0,0,0,0,0,0,0,0,0,0,0,-3566,4626,7707,4792,7726,5906,5906,7707,4631
DATA 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4608,4792,-18405,-3566,5934
DATA 4635,4626,6935,4631,0,0,0,0,0,0,0,0,0,0,0,0,0,4626,4626,-4334
DATA -16963,-16963,11794,4850,4626,-31011,5906,18,0,0,0,0,0,0,0,0,0,0,0,0
DATA 4608,7707,6942,4631,-16963,-17134,-3566,4626,4626,4626,4626,0,0,0,0,0,0,0,0,0
DATA 0,0,0,0,-31214,7154,7710,5915,-18414,-31043,4626,-8942,5906,5915,4742,0,0,0,0,0
DATA 0,0,0,0,0,0,0,0,4626,11794,7707,6942,4626,4626,4626,-803,5906,6939,4850,0
DATA 4626,18,0,0,0,0,0,0,0,0,0,0,4608,-3566,6958,6939,-3566,-8718,-772,12028
DATA 4742,-3561,4742,4626,6930,4626,18,0,0,0,0,0,0,0,0,0,4608,4626,12018,5911
DATA -3566,11822,11822,11822,4850,-30990,4626,4631,7698,6930,4631,0,0,0,0,0,0,0,0,0
DATA 4626,4626,-31214,-30990,-3566,2606,11806,-3538,4638,4626,-31214,4635,4631,7698,4635,0,0,0,0,0
DATA 0,0,0,4608,11910,4742,4626,4626,-3450,19186,11786,-3538,4618,4626,-3566,4638,5915,7698,4635,0
DATA 0,0,0,0,0,0,0,4608,-30990,-31214,-4334,4847,-31214,-3342,-31098,-31098,-31214,4829,-3450,4635
DATA 6942,7703,4631,0,0,0,0,0,0,0,0,-31214,7046,4631,4626,4847,-31214,-31098,6935,4635
DATA -8826,-772,-31214,6130,6930,5918,4626,0,0,0,0,0,0,0,0,4626,4638,4626,4631,-4334
DATA 5906,6939,7710,4631,-8826,-8708,4638,4626,4626,4626,18,0,0,0,0,0,0,0,0,6930
DATA 6930,4631,5906,-4334,4626,4626,4626,4626,-8826,-8708,7932,-8708,-8739,0,0,0,0,0,0,0
DATA 0,0,0,4608,7710,4635,4635,-4334,5906,7707,4626,4635,-8826,-8739,7932,-8708,-8960,0,0,0
DATA 0,0,0,0,0,0,0,4608,7707,6930,4631,4847,4626,7703,6942,4631,-31214,-8739,7932,-8708
DATA -1024,0,0,0,0,0,0,0,0,0,0,0,4626,5915,4626,4626,4626,6930,5915,4626
DATA 4626,-8826,-803,221,-1024,0,0,0,0,0,0,0,0,0,0,0,4608,4626,4626,4626
DATA 4626,4626,4626,4626,4626,4626,-8739,221,-1024,0,0,0,0,0,0,0,0,0,0,0
DATA 0,4608,4635,4626,4626,18,-31232,4626,4626,6930,-8942,221,-8960,0,0,0,0,0,0,0
DATA 0,0,0,0,0,7698,-31205,4626,18,0,-31098,4742,4626,7046,4631,221,-8960,0,0,0
DATA 0,0,0,0,0,0,0,0,4608,6942,11799,4742,0,-31098,0,-31098,-31214,7922,4635,0
DATA 221,0,0,0,0,0,0,0,0,0,0,0,-31214,5915,11822,4850,0,0,0,-31098
DATA -31214,-3342,6942,18,0,0,0,0,0,0,0,0,0,0,0,4608,-3363,11822,-3538,4850
DATA 0,0,-31232,-31098,-3566,11822,-8718,18,0,0,0,0,0,0,0,0,0,0,0,-8942
DATA 12028,11822,-3342,4742,0,0,-31098,4608,-3450,11822,-978,4829,0,0,0,0,0,0,0,0
DATA 0,0,4608,-803,-3332,-3342,-30990,4742,0,-31098,0,4608,-3450,11822,-803,-8708,18,0,0,0
DATA 0,0,0,0,0,0,4626,4626,-8708,-3342,-31098,18,0,0,0,4608,-3363,-978,4626,4626
DATA 18,0,0,0,0,0,0,0,0,4608,-3450,-30990,-1006,4829,-8942,18,0,0,0,4608
DATA -8942,4860,12018,-3342,4742,0,0,0,0,0,0,0,0,-3566,12018,-3538,-30990,7698,4635,0
DATA 0,0,0,4608,4635,-3566,11822,11822,-30990,18,0,0,0,0,0,0,4608,7707,5915,11794
DATA -3342,6930,4631,0,0,0,0,4608,4631,12018,5906,6939,5915,4742,0,0,0,0,0,0
DATA 6930,7710,6942,4631,-30990,4742,4626,0,0,0,0,4608,4626,4850,6935,7710,6942,4631,0,0
DATA 0,0,0,0,4626,4626,4626,4626,4626,4626,18,0,0,0,0,0,4626,4626,4626,4626
DATA 4626,4626,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
DATA 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
DATA 0,0,0,0,0,0,0,0,0,0,0,0,0,0

PaletteData: 'ZERO.PUT palette data.
DATA 0,2752512,10752,2763264,42,2752554,5418,2763306
DATA 1381653,4134165,1392405,4144917,1381695,4134207,1392447,4144959
DATA 0,328965,526344,723723,921102,1118481,1315860,1579032
DATA 1842204,2105376,2368548,2631720,2960685,3289650,3684408,4144959
DATA 0,4,8,12,16,21,25,29
DATA 33,37,42,46,50,54,58,63
DATA 0,262144,524288,786432,1048576,1376256,1638400,1900544
DATA 2162688,2424832,2752512,3014656,3276800,3538944,3801088,4128768
DATA 0,1024,2048,3072,4096,5376,6400,7424
DATA 8448,9472,10752,11776,12800,13824,14848,16128
DATA 0,262148,524296,786444,1048592,1376277,1638425,1900573
DATA 2162721,2424869,2752554,3014702,3276850,3538998,3801146,4128831
DATA 0,263168,526336,789504,1052672,1381632,1644800,1907968
DATA 2171136,2434304,2763264,3026432,3289600,3552768,3815936,4144896
DATA 0,1028,2056,3084,4112,5397,6425,7453
DATA 8481,9509,10794,11822,12850,13878,14906,16191
DATA 0,516,1032,1548,2064,2581,3097,3613
DATA 4385,4901,5418,5934,6450,6966,7482,8255
DATA 0,131076,262152,393228,524304,655381,786457,917533
DATA 1114145,1245221,1376298,1507374,1638450,1769526,1900602,2097215
DATA 0,262656,525312,787968,1050624,1378816,1641472,1904128
DATA 2167040,2429696,2757888,3020544,3283200,3545856,3808512,4136960
DATA 0,131844,263688,395532,527376,659477,791321,923165
DATA 1120545,1252389,1384490,1516334,1648178,1780022,1911866,2109503
DATA 0,131588,263176,394764,526352,657941,789529,921117
DATA 1118497,1250085,1381674,1513262,1644850,1776438,1908026,2105407
DATA 0,515,1030,1545,2060,2575,3090,3605
DATA 4376,4891,5406,5921,6436,6951,7466,8238
DATA 63,2103,4143,6183,8223,10008,12048,14088
DATA 16128,13833,11538,9243,6948,4653,2358,63
DATA 32,1058,2084,3110,4136,5418,6444,7470
DATA 8496,9522,10804,11830,12856,13882,14908,16191



SUB InitImageData (FileName$, ImageArray() as Ushort)

dim intcount as ushort
dim ints as ushort
dim fileno as ushort
dim n as ushort, x as ushort
    IF FileName$ <> "" THEN
        '***** Read image data from file *****

        'Establish size of integer array required.
        FileNo = FREEFILE
        OPEN FileName$ FOR BINARY AS #FileNo
        Ints = (LOF(FileNo) - 7) \ 2
        CLOSE #FileNo
        REDIM ImageArray(1 TO Ints) as ushort

        'Load image data directly into array memory.
        'DEF SEG = VARSEG(ImageArray(1))
        BLOAD FileName$, 0
        'DEF SEG
    ELSE
        '***** Read image data from DATA statements *****

        'Establish size of integer array required.
        READ IntCount
        REDIM ImageArray(1 TO IntCount) as Ushort

        'READ image DATA into array.
        FOR n = 1 TO IntCount
            READ X
            ImageArray(n) = X
        NEXT n
    END IF

END SUB

SUB MakeImageIndex (ImageArray() as ushort, IndexArray() as ushort)

    'The index will initially be built in a temporary array, allowing
    'for the maximum 1000 images per file.
    DIM Temp(1 TO 1000) as ushort
    dim xptr as integer
    dim IndexNo as integer
    dim Lastint as integer
    dim x as integer
    dim lastimage as integer
    dim n as integer
    xptr = 1: IndexNo = 1: LastInt = UBOUND(ImageArray)
    DO
        Temp(IndexNo) = xptr
        IndexNo = IndexNo + 1

        'Evaluate descriptor of currently referenced image to
        'calculate the beginning of the next image.
        X = (ImageArray(xptr) \ 8) * (ImageArray(xptr + 1)) + 4
        IF X MOD 2 THEN X = X + 1
        xptr = xptr + (X \ 2)
    LOOP WHILE xptr < LastInt

    LastImage = IndexNo - 1

    'Copy the image index values into the actual index array.
    REDIM IndexArray(1 TO LastImage) as USHORT
    FOR n = 1 TO LastImage
        IndexArray(n) = Temp(n)
    NEXT n
    ERASE Temp
END SUB

SUB LoadPalPp256 (File$) STATIC

'Loads a pp256 palette
'Changes the VGA palette on the fly
'if File$="" the data statement is used

dim c as uinteger
dim r as uinteger, g as uinteger, b as uinteger
dim n as Ushort, fr as integer
IF File$ = "" OR File$ = " " THEN
    FOR n = 0 TO 255
        READ C
        B = C \ 65536: C = C - B * 65536
        G = C \ 256: C = C - G * 256
        R = C
        palette n, r or (g shl 8) or (b shl 16)
     NEXT
ELSE
    FR = FREEFILE
    IF INSTR(File$, ".") = 0 THEN File$ = LEFT$(File$, 8) + ".Pal"
    OPEN File$ FOR BINARY AS #FR
        FOR n = 0 TO 255
            GET #FR, , C
            B = C \ 65536: C = C - B * 65536
            G = C \ 256: C = C - G * 256
            R = C
            OUT &H3C8, n
            OUT &H3C9, R
            OUT &H3C9, G
            OUT &H3C9, B
         NEXT
    CLOSE #FR
END IF

END SUB



