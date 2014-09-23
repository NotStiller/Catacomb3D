/* Catacomb 3-D SDL Port
 * Copyright (C) 2014 twitter.com/NotStiller
 * Copyright (C) 1993-2014 Flat Rock Software
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "srcport.h"

#include "id_heads.h"
#include "mapsc3d.h"

#define NUMSCALEWALLS 30

extern memptr			walldirectory[NUMSCALEWALLS];

int loadedmap=-1;
maptype mapheaderseg[30];

extern uint16_t maphead_RLEWtag;
extern int32_t maphead_headeroffsets[20];

pictabletype	*pictable;
pictabletype	*picmtable;
spritetabletype *spritetable;

static uint8_t *mapData=NULL;
static long mapDataSize=0;

static uint8_t *grData=NULL;
static long grDataSize=0;

static uint8_t *auData=NULL;
static long auDataSize=0;


// the following fields were extracted from _maphead, which in turn was 
// extracted from C3DMHEAD.OBJ.

uint16_t maphead_RLEWtag = 0xabcd;
int32_t maphead_headeroffsets[20] = {
     389,     851,    1568,    2159,
    2829,    3490,    4271,    5023,
    5316,    6018,    6753,    7416,
    8331,    9151,    9848,   10558,
   11353,   11948,   12682,   14246,
};


// The following data was extracted from the variables audiohead, audiodict, EGAhead, EGAdict, _maphead,
// which, in the original GPL release, were located in the respective .OBJ files.
int32_t GrChunksNum = 479;
int32_t GrChunksPos[479] = {
      0,    379,    393,    424,   1683,   2632,   2825,   3095,
   3288,   3476,   3769,   4120,   4337,   4637,   4940,   5194,
   5454,   5709,   5862,   5968,   6315,  25288,  40549,  54890,
  65238,  75705,  78631,  80471,  81964,  87247,  93222,  98412,
 102832, 107662, 112967, 117766, 123681, 123700, 123880, 124514,
 125346, 126231, 127331, 128448, 129560, 130647, 131709, 132805,
 133916, 135044, 136142, 137259, 138357, 139440, 140505, 141596,
 142695, 143805, 150613, 150628, 151159, 151718, 152259, 152813,
 153278, 153778, 154312, 154760, 155135, 155475, 156316, 157120,
 157919, 158744, 159562, 160571, 161620, 162577, 163458, 164086,
 164646, 165505, 166331, 167145, 167983, 168200, 168416, 168615,
 168816, 169051, 169271, 169507, 169739, 169961, 170279, 170788,
 170943, 171098, 171390, 171697, 172557, 173396, 174247, 175092,
 176018, 177060, 177938, 178847, 179855, 180587, 181190, 181946,
 182701, 183204, 183943, 184333, 184685, 185057, 185445, 185817,
 186184, 186529, 186808, 187879, 188951, 189758, 190544, 191231,
 191747, 192292, 192871, 193414, 193846, 194908, 194919, 195705,
 196604, 197508, 199037, 200068, 201065, 202547, 203466, 204330,
 205496, 207204, 207993, 209322, 210037, 210734, 211339, 212042,
 212899, 213768, 214784, 215801, 216658, 217527, 218392, 219264,
 220609, 223640, 224858, 226376, 226405, 226437, 226477, 227799,
 228084, 228101, 228186, 228293, 228365, 228483, 228552, 228594,
 228653, 228761, 228895, 228983, 229113, 229205, 229275, 229362,
 229458, 229562,     -1,     -1, 229651, 229752,     -1,     -1,
 229834, 229954,     -1,     -1, 230055, 230155,     -1,     -1,
 230236, 230337,     -1,     -1,     -1,     -1,     -1,     -1,
     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
     -1,     -1,     -1,     -1, 230419, 230466, 230549, 230635,
 230707, 230799, 230873, 230944, 231023, 231116, 231199, 231274,
 231365, 231434, 231528, 231622, 231714, 231793, 231882, 231965,
 232039, 232103, 232192, 232276, 232369, 232461, 232537, 232609,
 232656, 232703, 232750, 232797, 232844, 232891, 232938,     -1,
     -1, 232989, 233142, 233276, 233425, 233570, 233688, 233756,
 233854, 233940, 234049, 234136, 234223, 234378, 234532, 234689,
 234848, 234998, 235155, 235305, 235465, 235626, 235721, 235834,
 235941, 236095, 236143, 236242, 236319, 236462, 236577, 236691,
 236844, 236999, 237149,     -1, 237305, 237432, 237551, 237619,
 237719, 237812, 237938, 238058, 238126, 238228,     -1,     -1,
     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
 238321, 238704, 239073, 239582, 240062, 240569, 241051, 241540,
 242067, 242552, 242994, 243486, 243998, 244530, 245043, 245523,
 246031, 246533, 247049, 247569, 248166, 252502, 256899,
};

int32_t GrChunksSize[479] = {
    379,     14,     31,   1259,    949,    193,    270,    193,
    188,    293,    351,    217,    300,    303,    254,    260,
    255,    153,    106,    347,  18973,  15261,  14341,  10348,
  10467,   2926,   1840,   1493,   5283,   5975,   5190,   4420,
   4830,   5305,   4799,   5915,     19,    180,    634,    832,
    885,   1100,   1117,   1112,   1087,   1062,   1096,   1111,
   1128,   1098,   1117,   1098,   1083,   1065,   1091,   1099,
   1110,   6808,     15,    531,    559,    541,    554,    465,
    500,    534,    448,    375,    340,    841,    804,    799,
    825,    818,   1009,   1049,    957,    881,    628,    560,
    859,    826,    814,    838,    217,    216,    199,    201,
    235,    220,    236,    232,    222,    318,    509,    155,
    155,    292,    307,    860,    839,    851,    845,    926,
   1042,    878,    909,   1008,    732,    603,    756,    755,
    503,    739,    390,    352,    372,    388,    372,    367,
    345,    279,   1071,   1072,    807,    786,    687,    516,
    545,    579,    543,    432,   1062,     11,    786,    899,
    904,   1529,   1031,    997,   1482,    919,    864,   1166,
   1708,    789,   1329,    715,    697,    605,    703,    857,
    869,   1016,   1017,    857,    869,    865,    872,   1345,
   3031,   1218,   1518,     29,     32,     40,   1322,    285,
     17,     85,    107,     72,    118,     69,     42,     59,
    108,    134,     88,    130,     92,     70,     87,     96,
    104,     89,      0,      0,    101,     82,      0,      0,
    120,    101,      0,      0,    100,     81,      0,      0,
    101,     82,      0,      0,      0,      0,      0,      0,
      0,      0,      0,      0,      0,      0,      0,      0,
      0,      0,      0,      0,      0,      0,      0,      0,
      0,      0,      0,      0,      0,      0,      0,      0,
      0,      0,      0,      0,      0,      0,      0,      0,
      0,      0,      0,      0,      0,      0,      0,      0,
      0,      0,      0,      0,      0,      0,      0,      0,
      0,      0,      0,      0,      0,      0,      0,      0,
      0,      0,      0,      0,      0,      0,      0,      0,
      0,      0,      0,      0,      0,      0,      0,      0,
      0,      0,      0,      0,      0,      0,      0,      0,
      0,      0,      0,      0,      0,      0,      0,      0,
      0,      0,      0,      0,      0,      0,      0,      0,
      0,      0,      0,      0,      0,      0,      0,      0,
      0,      0,      0,      0,      0,      0,      0,      0,
      0,      0,      0,      0,      0,      0,      0,      0,
      0,      0,      0,      0,      0,      0,      0,      0,
      0,      0,      0,      0,      0,      0,      0,      0,
      0,      0,      0,      0,     47,     83,     86,     72,
     92,     74,     71,     79,     93,     83,     75,     91,
     69,     94,     94,     92,     79,     89,     83,     74,
     64,     89,     84,     93,     92,     76,     72,     47,
     47,     47,     47,     47,     47,     47,     51,      0,
      0,    153,    134,    149,    145,    118,     68,     98,
     86,    109,     87,     87,    155,    154,    157,    159,
    150,    157,    150,    160,    161,     95,    113,    107,
    154,     48,     99,     77,    143,    115,    114,    153,
    155,    150,    156,      0,    127,    119,     68,    100,
     93,    126,    120,     68,    102,     93,      0,      0,
      0,      0,      0,      0,      0,      0,      0,      0,
      0,      0,      0,      0,      0,      0,      0,      0,
      0,      0,      0,      0,      0,      0,      0,      0,
    383,    369,    509,    480,    507,    482,    489,    527,
    485,    442,    492,    512,    532,    513,    480,    508,
    502,    516,    520,    597,   4336,   4397,      0,
};

int32_t AudioChunksNum = 92;
int32_t AudioChunksPos[92] = {
      0,     16,    137,    234,    284,    331,    379,    432,
    491,    502,    538,    588,    638,    715,    749,    758,
    767,    838,    865,   1033,   1063,   1113,   1149,   1195,
   1237,   1246,   1295,   1340,   1351,   1360,   1395,   1421,
   1552,   1657,   1725,   1787,   1852,   1925,   2001,   2031,
   2079,   2174,   2239,   2413,   2463,   2482,   2501,   2553,
   2590,   2780,   2820,   2880,   2924,   2985,   3041,   3065,
   3137,   3198,   3222,   3246,   4284,   4284,   4284,   4284,
   4284,   4284,   4284,   4284,   4284,   4284,   4284,   4284,
   4284,   4284,   4284,   4284,   4284,   4284,   4284,   4284,
   4284,   4284,   4284,   4284,   4284,   4284,   4284,   4284,
   4284,   4284,   3300,   5062,
};

int32_t AudioChunksSize[92] = {
     16,    121,     97,     50,     47,     48,     53,     59,
     11,     36,     50,     50,     77,     34,      9,      9,
     71,     27,    168,     30,     50,     36,     46,     42,
      9,     49,     45,     11,      9,     35,     26,    131,
    105,     68,     62,     65,     73,     76,     30,     48,
     95,     65,    174,     50,     19,     19,     52,     37,
    190,     40,     60,     44,     61,     56,     24,     72,
     61,     24,     24,   1038,      0,      0,      0,      0,
      0,      0,      0,      0,      0,      0,      0,      0,
      0,      0,      0,      0,      0,      0,      0,      0,
      0,      0,      0,      0,      0,      0,      0,      0,
      0,   -984,   1762,      0,
};

huffnode audiohuffman[256] = {
{171, 173}, {256, 175}, {257, 185}, {258, 187}, {259, 206}, {260, 213}, {261, 215}, {262, 217}, 
{263, 220}, {264, 222}, {265, 231}, {266, 238}, {267, 248}, {268, 252}, {269, 254}, {270, 189}, 
{271, 192}, {201, 219}, {224, 227}, {229, 230}, {234,  89}, {110, 126}, {272, 176}, {194, 199}, 
{273, 208}, {274, 275}, {250,  93}, { 99, 101}, {102, 121}, {152, 172}, {174, 210}, {216, 221}, 
{223, 233}, {276, 236}, {243, 253}, { 82,  86}, { 87,  94}, {277, 120}, {134, 143}, {145, 157}, 
{169, 278}, {188, 279}, {280, 203}, {281, 225}, {237, 239}, {246,  61}, { 78,  95}, { 96, 106}, 
{108, 123}, {140, 151}, {154, 159}, {166, 170}, {191, 232}, {282,  70}, {283, 100}, {284, 107}, 
{113, 124}, {138, 144}, {285, 161}, {286, 287}, {218, 288}, {289, 235}, {290, 247}, {249,  90}, 
{ 91, 104}, {115, 127}, {131, 133}, {209, 226}, { 12,  29}, { 32,  57}, {291, 292}, {109, 293}, 
{114, 119}, {129, 130}, {294, 137}, {295, 146}, {296, 297}, {197, 298}, {204, 299}, {300, 244}, 
{ 26,  28}, { 69,  71}, { 92,  98}, {111, 156}, {164, 195}, {301,  27}, { 34,  58}, { 60,  65}, 
{ 66, 302}, {303, 304}, {305, 147}, {149, 153}, {306, 307}, {308, 202}, {214, 241}, { 20,  68}, 
{ 88, 141}, {168, 193}, {196, 205}, {309,  64}, {310, 311}, {312, 313}, {148, 314}, {315, 177}, 
{316, 317}, {318,  13}, {118, 165}, {167, 200}, {319,  62}, {320, 321}, {322, 323}, {212,  19}, 
{ 36,  59}, {125, 132}, {198, 255}, {324, 325}, { 41,  56}, { 72,  75}, {326, 327}, {328, 329}, 
{330, 135}, {331, 158}, {332, 333}, {334, 335}, {245,   7}, { 25,  35}, { 45,  73}, { 80, 128}, 
{136, 162}, {  8,  18}, {336,  37}, { 44, 337}, {338, 339}, {112, 160}, {340, 251}, {116, 341}, 
{342,  39}, { 55, 343}, {344, 345}, {346, 347}, {348, 349}, {350,  23}, { 51,  76}, {351,  21}, 
{ 38,  74}, {352, 353}, {180, 354}, {211,  33}, { 77, 103}, {117, 355}, { 24, 356}, {357, 142}, 
{358, 359}, {360,  81}, { 84, 181}, {207, 361}, {  9,  83}, {362, 363}, {186,  49}, { 53, 364}, 
{ 63, 365}, { 97, 366}, {139,   2}, {367, 368}, {369, 370}, { 47,  48}, { 79,  85}, {105, 150}, 
{155, 242}, {371,  16}, { 22, 372}, { 54, 373}, {374, 375}, {376, 377}, {378, 379}, {182, 380}, 
{381, 382}, {383, 384}, { 31, 228}, {385, 386}, {387, 388}, {389, 390}, { 17,  11}, { 14, 391}, 
{190, 240}, {392,  46}, {393, 394}, {395, 396}, {397, 398}, {399, 400}, {401, 402}, {403,   6}, 
{ 15,  50}, {404, 405}, {406,  40}, { 43, 407}, {408, 163}, {409, 410}, {411, 412}, {413, 414}, 
{415,  30}, {178,   5}, {416, 417}, {418, 419}, {420, 421}, {422, 423}, {424, 425}, {426, 427}, 
{428, 122}, {429, 430}, {  4, 431}, {432, 433}, {434,   3}, {435, 436}, {437, 183}, {438, 439}, 
{ 67,  52}, {440, 184}, {441, 442}, {443, 444}, {445, 446}, {  1, 447}, {448, 449}, {450, 451}, 
{452, 179}, {453, 454}, {455, 456}, { 10, 457}, {458, 459}, {460, 461}, {462, 463}, {464, 465}, 
{466, 467}, {468, 469}, {470,  42}, {471, 472}, {473, 474}, {475, 476}, {477, 478}, {479, 480}, 
{481, 482}, {483, 484}, {485, 486}, {487, 488}, {489, 490}, {491, 492}, {493, 494}, {495, 496}, 
{497, 498}, {499, 500}, {501, 502}, {503, 504}, {505, 506}, {507, 508}, {  0, 509}, {  0,   0}, 
};

huffnode grhuffman[256] = {
{210, 178}, {180, 150}, {214, 218}, {173,  37}, {256, 146}, { 73,  74}, {163, 169}, {165, 137}, 
{154, 181}, { 77, 145}, { 53, 211}, { 45,  91}, {172,  90}, {106, 139}, {141, 257}, { 89, 209}, 
{122, 166}, {258,  38}, {164, 149}, { 92, 203}, {212, 161}, {182,  75}, {259, 233}, {133, 157}, 
{ 66, 189}, { 52,  86}, {260, 174}, {261, 262}, { 94, 118}, { 26, 155}, {201,  22}, { 18, 237}, 
{263,  44}, { 54, 264}, {151, 265}, {185,  50}, {202, 266}, {183, 147}, {148,  46}, {197, 217}, 
{ 41, 167}, { 36, 267}, {268, 104}, {269,  43}, {270, 100}, {271,  35}, { 70, 272}, { 72, 221}, 
{220, 109}, { 58, 273}, { 88, 194}, {274, 188}, {177, 186}, {275,  82}, {229, 222}, { 93, 276}, 
{277, 278}, { 83, 279}, {280, 281}, {282,  33}, {283, 284}, {184, 285}, {215, 286}, {228, 113}, 
{142,  98}, {287, 288}, {289,  19}, { 81, 125}, {290,  69}, {291,  61}, { 67, 132}, {292, 293}, 
{226, 144}, { 59, 294}, {295, 296}, {117,  39}, {297,  71}, {205, 298}, {299, 300}, {301, 158}, 
{236,  20}, {302,  29}, {242, 303}, {130, 304}, {213, 200}, {305, 306}, {190, 136}, {244, 171}, 
{ 40, 162}, {208, 235}, {121,  65}, {307, 308}, {123, 176}, { 49, 309}, {230, 310}, { 17, 311}, 
{187, 107}, {312, 140}, {153, 313}, {134, 314}, {232, 234}, {315, 175}, { 27,  79}, {108, 316}, 
{  9, 138}, { 11, 119}, {317, 196}, {318, 238}, {319,  47}, {320, 216}, {103, 321}, { 97, 246}, 
{322,  55}, {323, 324}, { 34, 325}, {198,  25}, {245, 326}, { 87, 327}, {152, 328}, { 23, 329}, 
{330, 179}, {331, 332}, {333, 156}, {102,  21}, { 42, 126}, { 84, 334}, {135, 335}, {336, 337}, 
{110, 206}, {129, 338}, {250, 199}, {339,  14}, {340, 341}, {193, 225}, {342,  30}, {343, 344}, 
{ 57,  51}, {345, 120}, {116, 168}, { 80, 346}, {114, 347}, { 68,  56}, { 76, 348}, {124, 349}, 
{ 95,  28}, {350, 227}, {351, 101}, {352, 131}, { 62, 353}, {204, 354}, {355, 356}, {357, 241}, 
{358, 359}, {360, 361}, { 60, 362}, {363, 364}, {365, 366}, {367, 368}, {160, 195}, {369,  78}, 
{105, 370}, {371,  99}, {372, 373}, {115, 374}, {375,  12}, {376,   5}, { 13, 377}, {378, 379}, 
{380, 143}, {381, 207}, {382, 383}, {223, 384}, {385,  48}, {386, 387}, { 24, 388}, {389, 249}, 
{ 10, 390}, {159, 391}, {392, 393}, {243,   6}, {394, 111}, {395, 396}, {397, 398}, {399, 231}, 
{247, 400}, {401, 402}, {403, 219}, {404, 239}, {405, 406}, {407, 408}, {409, 410}, {191, 411}, 
{ 16, 412}, { 96, 413}, {414, 415}, {416, 417}, {418, 419}, {420,   2}, {421, 422}, {112, 253}, 
{423, 424}, {425,  64}, {426, 427}, {428, 429}, {430, 431}, {432,   4}, {433, 434}, {435, 436}, 
{251, 437}, {438, 439}, {  8, 440}, {441, 442}, {443, 444}, {445, 446}, {447, 448}, {449, 450}, 
{451, 452}, {252, 453}, { 63, 454}, { 31,  15}, {455,  32}, {240, 248}, {192, 224}, {456, 457}, 
{458, 127}, {459, 460}, {461,   7}, {462, 463}, {464,   1}, {  3, 465}, {466, 467}, {468, 469}, 
{128, 254}, {470,  85}, {471, 170}, {472, 473}, {474, 475}, {476, 477}, {478, 479}, {480, 481}, 
{482, 483}, {484, 485}, {486, 487}, {488, 489}, {490, 491}, {492, 493}, {494, 495}, {496, 497}, 
{498, 499}, {500, 501}, {502, 503}, {504, 505}, {506, 507}, {255, 508}, {  0, 509}, {  0,   0}, 
};

// end of extracted data


void CAL_HuffExpand (byte *source, byte *dest,long length,huffnode *hufftable);
void CAL_CarmackExpand (uint16_t *source, uint16_t *dest, long length);

uint8_t readU8(uint8_t **Buffer) {
	return *(*Buffer)++;
}

int8_t readS8(uint8_t **Buffer) {
	uint8_t v = *(*Buffer)++;
	return *(int8_t*)&v;
}

int16_t readU16(uint8_t **Buffer) {
	int16_t v = readU8(Buffer)+0x100*readU8(Buffer);
	return v;
}

int16_t readS16(uint8_t **Buffer) {
	uint16_t v = readU16(Buffer);
	return *(int16_t*)&v;
}

uint32_t readU32(uint8_t **Buffer) {
	return readU16(Buffer)+0x10000*readU16(Buffer);
}

int32_t readS32(uint8_t **Buffer) {
	uint32_t v = readU32(Buffer);
	return *(int32_t*)&v;
}

uint8_t *readFile(const char *Name, long *Size) {
	assert(Size != NULL);
	FILE *handle;
	handle = fopen(Name, "r");
	assert(handle != NULL);
	fseek(handle, 0, SEEK_END);
	*Size = ftell(handle);
	uint8_t *data = malloc(*Size);
	fseek(handle, 0, SEEK_SET);
	fread(data, 1, *Size, handle);
	//fclose(handle);

	return data;
}

static memptr readAndHuffExpand(uint8_t *Data, huffnode Dictionary[], long *Expanded) {	
	int32_t explen = readS32(&Data);
	if (Expanded) {
		*Expanded = explen;
	}
	memptr buffer = malloc(explen);
	CAL_HuffExpand (Data, buffer,explen,Dictionary);
	assert(buffer != NULL);
	return buffer;
}



static void deplaneRaw (memptr Source, int Width, int Height, uint8_t *Buffer) {
	byte *plane0,*plane1,*plane2,*plane3;
	plane0 = Source;
	plane1 = plane0 + Width*Height;
	plane2 = plane1 + Width*Height;
	plane3 = plane2 + Width*Height;

	int x,y;
	for (y=0;y<Height;y++)
	{
		for (x=0;x<Width;x++)
		{
			byte by0,by1,by2,by3;
			by0 = *plane0++;
			by1 = *plane1++;
			by2 = *plane2++;
			by3 = *plane3++;

			int b;
			for (b=0;b<8;b++)
			{
				Buffer[7-b] = ((by3&1)<<3) + ((by2&1)<<2) + ((by1&1)<<1) + (by0&1);
				by3 >>= 1;
				by2 >>= 1;
				by1 >>= 1;
				by0 >>= 1;
			}
			Buffer += 8;
		}
	}
}


static void deplaneMaskedRaw(memptr Source, int Width, int Height, uint8_t *Buffer) {
	byte *plane0,*plane1,*plane2,*plane3,*planeM;
	planeM = Source;
	plane0 = planeM + Width*Height;
	plane1 = plane0 + Width*Height;
	plane2 = plane1 + Width*Height;
	plane3 = plane2 + Width*Height;

	int x,y;
	for (y=0;y<Height;y++)
	{
		for (x=0;x<Width;x++)
		{
			byte byM,by0,by1,by2,by3;
			byM = *planeM++;
			by0 = *plane0++;
			by1 = *plane1++;
			by2 = *plane2++;
			by3 = *plane3++;

			int b;
			for (b=0;b<8;b++)
			{
				if (byM&1) {
					Buffer[7-b] = 0xFF;
				} else {
					Buffer[7-b] = ((by3&1)<<3) + ((by2&1)<<2) + ((by1&1)<<1) + (by0&1);
				}				
				by3 >>= 1;
				by2 >>= 1;
				by1 >>= 1;
				by0 >>= 1;
				byM >>= 1;
			}
			Buffer += 8;
		}
	}
}



static pictabletype *loadPicTable(uint8_t *Data, long DataSize, int NumElements) {
	memptr buffer = readAndHuffExpand(Data, grhuffman, NULL);
	pictabletype *table = malloc(sizeof(pictabletype)*NumElements);
	int i;
	uint8_t *p = buffer;
	for (i = 0; i < NumElements; i++) {
		table[i].width = readS16(&p);
		table[i].height = readS16(&p);
	}
	free(buffer);
	return table;
}


static spritetabletype *loadSpriteTable(uint8_t *Data, long DataSize, int NumElements) {
	memptr buffer = readAndHuffExpand(Data, grhuffman, NULL);

	spritetabletype *table = malloc(sizeof(spritetabletype)*NumElements);
	int i;
	uint8_t *p = buffer;
	for (i = 0; i < NumElements; i++) {
		table[i].width = readS16(&p);
		table[i].height = readS16(&p);
		table[i].orgx = readS16(&p);
		table[i].orgy = readS16(&p);
		table[i].xl = readS16(&p);
		table[i].yl = readS16(&p);
		table[i].xh = readS16(&p);
		table[i].yh = readS16(&p);
		table[i].shifts = readS16(&p);
	}
	free(buffer);
	return table;
}

static void deplanePic (memptr Source, int Width, int Height, boolean Masked, memptr *Destination)
{
	MM_GetPtr(Destination, 8*Width*Height+2*sizeof(uint32_t));
	((uint32_t*)*Destination)[0] = 8*Width;
	((uint32_t*)*Destination)[1] = Height;
	if (Masked) {
		deplaneMaskedRaw(Source, Width, Height, *Destination+2*sizeof(uint32_t));
	} else {
		deplaneRaw(Source, Width, Height, *Destination+2*sizeof(uint32_t));
	}
}

static void loadPic(int Chunk) {
	memptr buffer = readAndHuffExpand(grData+GrChunksPos[Chunk], grhuffman, NULL);
	deplanePic(buffer, pictable[Chunk-STARTPICS].width, pictable[Chunk-STARTPICS].height, false, &grsegs[Chunk]);
	free(buffer);
}

static void loadMaskedPic(int Chunk) {
	memptr buffer = readAndHuffExpand(grData+GrChunksPos[Chunk], grhuffman, NULL);
	deplanePic(buffer, picmtable[Chunk-STARTPICM].width, picmtable[Chunk-STARTPICM].height, true, &grsegs[Chunk]);
	free(buffer);
}

static void loadSprite(int Chunk) {
// This is a crippled version of the sprite loading function, but we need no bit
// shifting functionality anyway. Now sprites are just replaced by masked pics,
// which they really are.
	memptr buffer = readAndHuffExpand(grData+GrChunksPos[Chunk], grhuffman, NULL);
	spritetabletype *spr = &spritetable[Chunk-STARTSPRITES];
	assert(spr->orgx == 0);
	assert(spr->orgy == 0);
	assert(spr->shifts == 4);
	deplanePic(buffer, spr->width, spr->height, true, &grsegs[Chunk]);
	free(buffer);
}

void loadTile(int Chunk, int NumTilesInChunk, int Width, int Height, boolean Masked) {
	assert(grsegs[Chunk] == NULL);
	
	long pos = GrChunksPos[Chunk];
	assert(pos >= 0);
	
	int srcSpriteSize = (Masked?5:4)*Width*Height,
		dstSpriteSize = 8*Width*Height;
	uint8_t *buffer = malloc(NumTilesInChunk*srcSpriteSize);
	CAL_HuffExpand(grData+pos,buffer,NumTilesInChunk*srcSpriteSize,grhuffman);
	MM_GetPtr (&grsegs[Chunk],NumTilesInChunk*dstSpriteSize);
	int i;
	for (i = 0; i < NumTilesInChunk; i++) {
		if (!Masked) {
			deplaneRaw(buffer+i*srcSpriteSize, Width, Height, (uint8_t*)grsegs[Chunk]+i*dstSpriteSize);
		} else {
			deplaneMaskedRaw(buffer+i*srcSpriteSize, Width, Height, (uint8_t*)grsegs[Chunk]+i*dstSpriteSize);
		}
	}
	free(buffer);
}

static void loadFont(int Chunk) {
	long explen;
	memptr buffer = readAndHuffExpand(grData+GrChunksPos[Chunk], grhuffman, &explen);
	int datalen = explen-3*256-2;
	uint8_t *p = (uint8_t*)buffer;
	MM_GetPtr(&grsegs[Chunk], datalen+sizeof(fontstruct));
	fontstruct *font = grsegs[Chunk];
	font->height = readS16(&p);
	int i;
	for (i = 0; i < 256; i++) {
		font->location[i] = readS16(&p)-3*256-2+sizeof(fontstruct);
	}
	for (i = 0; i < 256; i++) {
		uint8_t w = readU8(&p);
		assert(w < 128);
		font->width[i] = (char)w;
	}
	memcpy((uint8_t*)font+sizeof(fontstruct), p, datalen);
	free(buffer);
}

static void loadMapTexts(int Chunk, int Level) {
	long len=0;
	memptr buffer = readAndHuffExpand(grData+GrChunksPos[Chunk], grhuffman, &len);
	MM_GetPtr(&grsegs[Chunk], len); // just to make the accountant shut up, erase this later !
	memcpy(grsegs[Chunk], buffer, len);
	free(buffer);

	int     i;
	char *p = grsegs[Chunk];
	mapheaderseg[Level].texts[0] = NULL;
	for (i=1;i<27;i++)
	{
		while (*p != '\n')
		{
			if (*p == '\r')
				*p = 0;
			p++;
		}
		p++;
		mapheaderseg[Level].texts[i] = p;
	}
}

static void loadMapHeader(uint8_t *Data, int Map) {
	int mapnum = Map;

// load map header
	{
		long	pos;
		pos = maphead_headeroffsets[mapnum];
		assert(pos >= 0);
		uint8_t *data = Data+pos;

		maptype *map = &mapheaderseg[mapnum];
		byte *p = data;
		map->planestart[0] = readS32(&p);
		map->planestart[1] = readS32(&p);
		map->planestart[2] = readS32(&p);
		map->planelength[0] = readU16(&p);
		map->planelength[1] = readU16(&p);
		map->planelength[2] = readU16(&p);
		map->width = readU16(&p);
		map->height = readU16(&p);
		int i;
		for (i = 0; i < 16; i++) {
			map->name[i] = readS8(&p);
		}
	}

	int size = mapheaderseg[mapnum].width * mapheaderseg[mapnum].height * 2;

	int plane;
	for (plane = 0; plane<MAPPLANES; plane++)
	{
		mapheaderseg[mapnum].rawplaneslength[plane] = 0;
		long pos = mapheaderseg[mapnum].planestart[plane];
		long compressed = mapheaderseg[mapnum].planelength[plane];		
		if (!compressed)
			continue;		// the plane is not used in this game

		memptr *dest = (memptr*)&mapheaderseg[mapnum].mapsegs[plane];
		MM_GetPtr(dest,size);

		int16_t *source = (int16_t*)(Data+pos);
		long expanded = *source;
		source++;
		char *buffer = (char*)malloc(expanded);

		CAL_CarmackExpand (source, (uint16_t*)buffer,expanded);
		CA_RLEWexpand ((uint16_t*)buffer+1,*dest,size,maphead_RLEWtag);
		free(buffer);
		mapheaderseg[mapnum].rawplaneslength[plane] = size;
		
		MM_GetPtr(&mapheaderseg[mapnum].rawplanes[plane], mapheaderseg[mapnum].rawplaneslength[plane]);
		memcpy(mapheaderseg[mapnum].rawplanes[plane], mapheaderseg[mapnum].mapsegs[plane], mapheaderseg[mapnum].rawplaneslength[plane]);
	}
	int i;
	for (i = 0; i < 27; i++) {
		mapheaderseg[mapnum].texts[i] = NULL;
	}
}


static void loadTables(uint8_t *Data) {
	pictable = loadPicTable(Data+GrChunksPos[STRUCTPIC], GrChunksSize[STRUCTPIC], NUMPICS);
	picmtable = loadPicTable(Data+GrChunksPos[STRUCTPICM], GrChunksSize[STRUCTPICM], NUMPICM);
	spritetable = loadSpriteTable(Data+GrChunksPos[STRUCTSPRITE], GrChunksSize[STRUCTSPRITE], NUMSPRITES);
}

static void setupScaleWall(int Pic) {
	int scnum = Pic-FIRSTWALLPIC;
	uint8_t *buffer = grsegs[Pic], *p = buffer;
	int width = readU32(&p);
	int height = readU32(&p);
	assert(width <= 64);
	assert(height <= 64);
	if (width > 64) {
		width = 64;
	}
	if (height > 64) {
		height = 64;
	}
	MM_GetPtr(&walldirectory[scnum],64*64);
	uint8_t *dest = walldirectory[scnum];
	int x,y;
	for (x=0;x<width;x++)
		for (y=0;y<height;y++)
			*dest++ = p[y*width+x];
}

static void setupScalePic (int Pic) {
	uint8_t *buffer = grsegs[Pic], *p = buffer;
	int width = readU32(&p);
	int height = readU32(&p);
	assert(width <= 64);
	assert(height <= 64);
	uint8_t dest[64*64];
	memset(dest, 5, 64*64);
	int x,y;
	for (y=0;y<height;y++)
		for (x=0;x<width;x++)
			dest[y*64+x] = p[y*width+x];
	BuildCompShape (Pic-FIRSTSCALEPIC, dest);
}


void loadAdlibSounds(uint8_t *Data) {
	SPA_InitSamples(NUMSOUNDS, 1);

	int i;
	for (i = 0; i < NUMSOUNDS; i++) {
		memptr buffer = readAndHuffExpand(Data+AudioChunksPos[STARTADLIBSOUNDS+i], audiohuffman, NULL);
		SoundCommon *s =  buffer;
		SPA_RenderSample(i, buffer);
		free(buffer);
	}
	{
		int i=0;
		memptr buffer = readAndHuffExpand(Data+AudioChunksPos[STARTMUSIC+i], audiohuffman, NULL);
		SPA_RenderMusic(i, buffer);
		free(buffer);
	}
}

static void loadMapHeaders(uint8_t *Data) {
	memset(&mapheaderseg, 0, sizeof(mapheaderseg));
	int i;
	for (i = APPROACH_MAP; i < NEMESISSLAIR_MAP; i++) {
		loadMapHeader(Data, i);
	}	
}

void SPD_SetupCatacomb3DData() {
	mapData = readFile("GAMEMAPS.C3D", &mapDataSize);
	auData = readFile("AUDIO.C3D", &auDataSize);
	grData = readFile("EGAGRAPH.C3D", &grDataSize);

	memset(grsegs, 0, sizeof(grsegs));

	loadMapHeaders(mapData);
	loadAdlibSounds(auData);
	loadTables(grData);
}


void combinedLoader(int Chunk) {
	if (Chunk >= STARTFONT && Chunk < STARTFONT+NUMFONT) {
		loadFont(Chunk);
	} else if (Chunk >= STARTPICS && Chunk < STARTPICS+NUMPICS) {
		loadPic(Chunk);
	} else if (Chunk >= STARTPICM && Chunk < STARTPICM+NUMPICM) {
		loadMaskedPic(Chunk);
	} else if (Chunk >= STARTSPRITES && Chunk < STARTSPRITES+NUMSPRITES) {
		loadSprite(Chunk);
	} else if (Chunk >= STARTSPRITES && Chunk < STARTSPRITES+NUMSPRITES) {
		loadSprite(Chunk);
	} else if (Chunk == STARTTILE8) {
		loadTile(STARTTILE8, NUMTILE8, 1, 8, false);
	} else if (Chunk == STARTTILE8M) {
		loadTile(STARTTILE8M, NUMTILE8M, 1, 8, true);
	} else if (Chunk >= STARTTILE16 && Chunk < STARTTILE16+NUMTILE16) {
		loadTile(Chunk, 1, 2, 16, false);
	} else if (Chunk >= STARTTILE16M && Chunk < STARTTILE16M+NUMTILE16M) {
		loadTile(Chunk, 1, 2, 16, true);
	} else if (Chunk >= STARTTILE32 && Chunk < STARTTILE32+NUMTILE32) {
		loadTile(Chunk, 1, 4, 32, false);
		assert(false);
	} else if (Chunk >= STARTTILE32M && Chunk < STARTTILE32M+NUMTILE32M) {
		loadTile(Chunk, 1, 4, 32, true);
		assert(false);
	} else if (Chunk >= LEVEL1TEXT && Chunk <= LEVEL20TEXT) {
		printf("MAP TEXTS\n");
		loadMapTexts(Chunk,Chunk-LEVEL1TEXT);
	} else {
		assert(false);
	}
}

void SPD_SetupScaleWall(int Chunk) {
	loadPic(Chunk);
	setupScaleWall(Chunk);
}

void SPD_SetupScalePic(int Chunk) {
	loadPic(Chunk);
	setupScalePic(Chunk);
}


void SPD_LoadGrChunk(int Chunk) {
	if (grsegs[Chunk]) {
		return;
	}
	if (GrChunksPos[Chunk] < 0) {
		return;
	}
	combinedLoader(Chunk);
}


void SPD_LoadMap(mapnum) {
	if (loadedmap != -1) {
		int plane;
		for (plane=0;plane<MAPPLANES;plane++)
			if (mapheaderseg[loadedmap].mapsegs[plane])
				MM_FreePtr (&mapheaderseg[loadedmap].mapsegs[plane]);
	}

	loadedmap = mapnum;

// load the planes in
// If a plane's pointer still exists it will be overwritten (levels are
// allways reloaded, never cached)

	int size = mapheaderseg[mapnum].width * mapheaderseg[mapnum].height * 2;

	int plane;
	for (plane = 0; plane<MAPPLANES; plane++)
	{
		if (!mapheaderseg[mapnum].rawplaneslength[plane])
			continue;
		MM_GetPtr (&mapheaderseg[loadedmap].mapsegs[plane], size);
		memcpy(mapheaderseg[mapnum].mapsegs[plane], mapheaderseg[mapnum].rawplanes[plane], mapheaderseg[mapnum].rawplaneslength[plane]);
	}
	loadMapTexts(LEVEL1TEXT+mapnum,mapnum);
}

