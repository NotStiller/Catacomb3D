/* Catacomb 3-D SDL Port
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
#include "sp_data_int.h"
#include "c4_def.h"

const char *GamespecificExtension = "ABS";

pictabletype	*pictable;
pictabletype	*picmtable;
spritetabletype *spritetable;

const int StartFont = STARTFONT;
const int StartTile8 = STARTTILE8;
const int StartTile8M = STARTTILE8M;


// EAST / WEST WALLS
//
int	walllight1[NUMFLOORS] = {0,
	W_WARP1EWPIC,
	W_NEMPICEWPIC,W_PENTAEWPIC,W_ALTER_LFPIC,W_ALTER_RTPIC,
	W_SUB1EWPIC,W_SUB2EWPIC,W_SUB3EWPIC,

	W_TORCH1PIC,W_TORCH2PIC,
	W_LSUB_STONEPIC,
	W_BLOODY_LSUB_STONEPIC,
	W_BREATH_LWALL1PIC,W_BREATH_LWALL2PIC,

	EXPWALL1PIC,EXPWALL2PIC,EXPWALL3PIC,

	W_WOOD_DOORWAYPIC,W_WOOD_DOORWAY_GLOWPIC,

	W_WATER1EWPIC,W_DRAIN1EWPIC,
	W_WATER2EWPIC,W_DRAIN2EWPIC,

	W_WOODEN_DOORPIC,W_WOOD_DOOREWPIC,W_METAL_DOORPIC,W_GLOW_DOORPIC,

	W_FINALEXITPIC,

	W_WATER_EXP1PIC,W_WATER_EXP2PIC,W_WATER_EXP3PIC,

	W_PRE_CHEATSPIC,W_CHEAT_WARPPIC,W_CHEAT_FREEZEPIC,W_SURFACE_PLAQPIC,

	W_WATER_GATEEW1PIC,
	WALL8LPIC,WALL9LPIC,WALL10DPIC,WALL11LPIC,WALL12LPIC,WALL13LPIC,WALL14LPIC,WALL15LPIC,
	WALL16LPIC,WALL17LPIC,W_WINDOWEWPIC,WALL19LPIC,WALL20LPIC,WALL21LPIC,
	WALL22LPIC,WALL23LPIC,WALL24LPIC,WALL25LPIC,WALL26LPIC,WALL27LPIC,
	WALL28LPIC,WALL29LPIC,WALL30LPIC,WALL31LPIC,
	W_BREATH_LWALL4PIC,W_BREATH_LWALL3PIC,
	MAGE_STATUEPIC,ZOMBIE_STATUEPIC,EYE_STATUEPIC,NEM_STATUEPIC,
	SKELETON_STATUEPIC,SPOOK_STATUEPIC,ORCH_STATUEPIC,
	};

// NORTH / SOUTH WALLS
//
int	walldark1[NUMFLOORS] = {0,
	W_WARP1NSPIC,
	W_NEMPICEWPIC,W_PENTANSPIC,1,1,
	W_SUB1NSPIC,W_SUB2NSPIC,W_SUB3NSPIC,

	W_TORCH1PIC,W_TORCH2PIC,
	W_DSUB_STONEPIC,
	W_BLOODY_DSUB_STONEPIC,
	W_BREATH_DWALL1PIC,W_BREATH_DWALL2PIC,

	EXPWALL1PIC,EXPWALL2PIC,EXPWALL3PIC,

	W_WOOD_DOORWAYPIC,W_WOOD_DOORWAY_GLOWPIC,

	W_WATER1NSPIC,W_DRAIN1NSPIC,
	W_WATER2NSPIC,W_DRAIN2NSPIC,
	W_WOODEN_DOORPIC,W_WOOD_DOORNSPIC,W_METAL_DOORPIC,W_GLOW_DOORPIC,

	W_FINALEXITPIC,

	W_WATER_EXP1PIC,W_WATER_EXP2PIC,W_WATER_EXP3PIC,

	W_CHEAT_GODPIC,W_CHEAT_ITEMSPIC,W_POST_CHEATPIC,W_SURFACE_PLAQPIC,

	W_WATER_GATENS1PIC,
	WALL8DPIC,WALL9DPIC,WALL10LPIC,WALL11DPIC,WALL12DPIC,WALL13DPIC,WALL14DPIC,WALL15DPIC,
	WALL16DPIC,WALL17DPIC,W_WINDOWNSPIC,WALL19DPIC,WALL20DPIC,WALL21DPIC,
	WALL22DPIC,WALL23DPIC,WALL24DPIC,WALL25DPIC,WALL26DPIC,WALL27DPIC,
	WALL28DPIC,WALL29DPIC,WALL30DPIC,WALL31DPIC,
	W_BREATH_DWALL4PIC,W_BREATH_DWALL3PIC,
	MAGE_STATUEPIC,ZOMBIE_STATUEPIC,EYE_STATUEPIC,NEM_STATUEPIC,
	SKELETON_STATUEPIC,SPOOK_STATUEPIC,ORCH_STATUEPIC,
	};

// EAST / WEST WALLS
//
int	walllight2[NUMFLOORS] = {0,
	W_WARP2EWPIC,
	W_NEMPICEWPIC,W_PENTAEWPIC,W_ALTER_LFPIC,W_ALTER_RTPIC,
	W_SUB1EWPIC,W_SUB2EWPIC,W_SUB3EWPIC,

	W_TORCH2PIC,W_TORCH1PIC,
	W_LSUB_STONEPIC,
	W_BLOODY_LSUB_STONEPIC,
	W_BREATH_LWALL2PIC,W_BREATH_LWALL1PIC,

	EXPWALL2PIC,EXPWALL1PIC,EXPWALL3PIC,

	W_WOOD_DOORWAYPIC,W_WOOD_DOORWAY_GLOWPIC,

	W_WATER2EWPIC,W_DRAIN2EWPIC,
	W_WATER1EWPIC,W_DRAIN1EWPIC,

	W_WOODEN_DOORPIC,W_WOOD_DOOREWPIC,W_METAL_DOORPIC,W_GLOW_DOORPIC,

	W_FINALEXITPIC,

	W_WATER_EXP2PIC,W_WATER_EXP1PIC,W_WATER_EXP1PIC,

	W_PRE_CHEATSPIC,W_CHEAT_WARPPIC,W_CHEAT_FREEZEPIC,W_SURFACE_PLAQPIC,

	W_WATER_GATEEW2PIC,
	WALL8LPIC,WALL9LPIC,WALL10DPIC,WALL11LPIC,WALL12LPIC,WALL13LPIC,WALL14LPIC,WALL15LPIC,
	WALL16LPIC,WALL17LPIC,W_WINDOWEWPIC,WALL19LPIC,WALL20LPIC,WALL21LPIC,
	WALL22LPIC,WALL23LPIC,WALL24LPIC,WALL25LPIC,WALL26LPIC,WALL27LPIC,
	WALL28LPIC,WALL29LPIC,WALL30LPIC,WALL31LPIC,
	W_BREATH_LWALL3PIC,W_BREATH_LWALL4PIC,
	MAGE_STATUEPIC,ZOMBIE_STATUEPIC,EYE_STATUEPIC,NEM_STATUEPIC,
	SKELETON_STATUEPIC,SPOOK_STATUEPIC,ORCH_STATUEPIC,
	};

// NORTH / SOUTH WALLS
//
int	walldark2[NUMFLOORS] = {0,
	W_WARP2NSPIC,
	W_NEMPICEWPIC,W_PENTANSPIC,1,1,
	W_SUB1NSPIC,W_SUB2NSPIC,W_SUB3NSPIC,

	W_TORCH2PIC,W_TORCH1PIC,
	W_DSUB_STONEPIC,
	W_BLOODY_DSUB_STONEPIC,
	W_BREATH_DWALL2PIC,W_BREATH_DWALL1PIC,

	EXPWALL2PIC,EXPWALL1PIC,EXPWALL3PIC,

	W_WOOD_DOORWAYPIC,W_WOOD_DOORWAY_GLOWPIC,

	W_WATER2NSPIC,W_DRAIN2NSPIC,
	W_WATER1NSPIC,W_DRAIN1NSPIC,

	W_WOODEN_DOORPIC,W_WOOD_DOORNSPIC,W_METAL_DOORPIC,W_GLOW_DOORPIC,

	W_FINALEXITPIC,

	W_WATER_EXP2PIC,W_WATER_EXP1PIC,W_WATER_EXP1PIC,

	W_CHEAT_GODPIC,W_CHEAT_ITEMSPIC,W_POST_CHEATPIC,W_SURFACE_PLAQPIC,

	W_WATER_GATENS2PIC,
	WALL8DPIC,WALL9DPIC,WALL10LPIC,WALL11DPIC,WALL12DPIC,WALL13DPIC,WALL14DPIC,WALL15DPIC,
	WALL16DPIC,WALL17DPIC,W_WINDOWNSPIC,WALL19DPIC,WALL20DPIC,WALL21DPIC,
	WALL22DPIC,WALL23DPIC,WALL24DPIC,WALL25DPIC,WALL26DPIC,WALL27DPIC,
	WALL28DPIC,WALL29DPIC,WALL30DPIC,WALL31DPIC,
	W_BREATH_DWALL3PIC,W_BREATH_DWALL4PIC,
	MAGE_STATUEPIC,ZOMBIE_STATUEPIC,EYE_STATUEPIC,NEM_STATUEPIC,
	SKELETON_STATUEPIC,SPOOK_STATUEPIC,ORCH_STATUEPIC,
	};



// extracted data

huffnode C4_audiohuffman[256] = {
{171, 173}, {256, 175}, {257, 185}, {258, 187}, 
{259, 206}, {260, 208}, {261, 213}, {262, 215}, 
{263, 217}, {264, 220}, {265, 222}, {266, 231}, 
{267, 238}, {268, 248}, {269, 250}, {270, 252}, 
{271, 253}, {272, 254}, {273, 189}, {274, 224}, 
{225, 227}, {229, 234}, {247, 275}, {194, 199}, 
{218, 276}, {226, 277}, {230,  89}, {201, 210}, 
{221, 233}, {236, 278}, {251,  87}, {279, 203}, 
{214, 216}, {280, 281}, {237, 133}, {134, 143}, 
{161, 166}, {219, 282}, {232,  94}, {140, 145}, 
{154, 283}, {284, 285}, {243, 246}, {124, 147}, 
{169, 286}, { 82, 113}, {152, 287}, {202, 288}, 
{289, 245}, { 59,  73}, {131, 159}, {174, 188}, 
{190, 209}, {290,  78}, {101, 120}, {126, 129}, 
{291, 141}, {292, 176}, {191, 204}, {293, 223}, 
{244, 121}, {142, 157}, {164, 168}, {192, 195}, 
{196, 205}, {294,  12}, { 13,  96}, {108, 130}, 
{295, 296}, {197, 297}, {298,  61}, {104, 110}, 
{127, 136}, {144, 106}, {299, 137}, {138, 158}, 
{300, 193}, {211,   8}, { 24,  71}, { 93, 115}, 
{151, 177}, {200, 301}, { 91, 100}, {117, 146}, 
{149, 302}, {303, 304}, {235, 241}, {249, 255}, 
{ 19,  55}, { 75, 114}, {172, 212}, {305,  66}, 
{ 68,  92}, { 95, 107}, {116, 306}, {160, 307}, 
{308,  70}, { 99, 111}, {155, 165}, {309,  28}, 
{ 60, 310}, {119, 311}, {312, 313}, {314, 315}, 
{239,  36}, { 37,  53}, { 58,  88}, {125, 153}, 
{316,   7}, { 77,  86}, { 98, 135}, {317, 318}, 
{319, 320}, {  9,  45}, { 64, 109}, {321, 322}, 
{ 29,  80}, {323, 112}, {132, 324}, {162, 170}, 
{325, 167}, {182, 326}, { 21,  31}, { 74, 327}, 
{328, 128}, { 23,  62}, { 69, 123}, {329,  27}, 
{ 51, 103}, {330, 331}, {332,  32}, { 44, 333}, 
{228, 334}, { 65, 335}, {102, 336}, {186,  72}, 
{ 81,  90}, {337,  57}, {338, 339}, {139, 340}, 
{150, 341}, {342, 343}, { 35,  38}, { 54, 198}, 
{ 16, 344}, {345, 346}, {180,  18}, { 20,  39}, 
{ 52,  56}, { 83,  97}, {347, 348}, {349, 350}, 
{351,  79}, {352,  25}, { 48,  50}, { 84, 353}, 
{354,  11}, {355,  15}, { 33, 356}, {357, 358}, 
{359, 360}, { 30, 361}, { 47, 362}, {363, 148}, 
{181, 207}, {364,  34}, {365, 366}, {367, 368}, 
{  2,  40}, {369, 370}, {240,  26}, {371, 372}, 
{373, 374}, {375,  85}, {105, 376}, { 41, 377}, 
{ 46, 378}, {379, 380}, {242, 381}, {382, 383}, 
{163, 178}, {384, 385}, {  4, 386}, {  6,  17}, 
{387, 118}, {388, 389}, {390, 391}, { 14, 392}, 
{156, 393}, {394, 395}, {396, 397}, {398, 399}, 
{122, 400}, {401, 402}, {403, 404}, {405,   3}, 
{ 76, 406}, { 63, 407}, {408, 409}, {410, 411}, 
{ 22, 412}, {  5,  43}, {413, 414}, {415, 184}, 
{416, 417}, {418,  67}, {419, 420}, {421, 422}, 
{423, 183}, {424, 425}, {426, 427}, {428, 429}, 
{430, 431}, {432, 433}, {434, 435}, {436, 179}, 
{437, 438}, {439, 440}, {441, 442}, {443,  49}, 
{444,   1}, {445, 446}, {447, 448}, {449, 450}, 
{451,  10}, {452, 453}, {454, 455}, {456, 457}, 
{458, 459}, {460, 461}, { 42, 462}, {463, 464}, 
{465, 466}, {467, 468}, {469, 470}, {471, 472}, 
{473, 474}, {475, 476}, {477, 478}, {479, 480}, 
{481, 482}, {483, 484}, {485, 486}, {487, 488}, 
{489, 490}, {491, 492}, {493, 494}, {495, 496}, 
{497, 498}, {499, 500}, {501, 502}, {503, 504}, 
{505, 506}, {507,   0}, {508, 509}, {  0,   0}, 
};

huffnode C4_grhuffman[256] = {
{ 45, 209}, {169, 150}, {172, 162}, { 89,  75}, 
{141,  53}, {154, 178}, {139,  77}, {164, 229}, 
{173, 210}, { 90, 202}, { 78, 163}, {166,  86}, 
{149, 146}, {177, 157}, { 91, 212}, {133, 185}, 
{165, 203}, { 73, 148}, { 82, 106}, { 69, 180}, 
{145, 201}, { 94, 161}, {138,  41}, {218,  83}, 
{ 37, 147}, {256,  43}, {137,  92}, {211,  58}, 
{179,  81}, {151, 257}, {107,  52}, { 44, 155}, 
{197, 258}, {158,  74}, {186,  93}, { 70,  50}, 
{174, 181}, {259, 233}, {205, 182}, { 54, 217}, 
{260, 167}, {261, 234}, {262, 168}, {189,  46}, 
{263, 213}, {184, 153}, { 98, 264}, {100, 171}, 
{ 76, 265}, { 57,  59}, {266, 121}, { 61, 267}, 
{156, 118}, {142, 268}, { 21, 269}, {226, 270}, 
{271, 272}, {194, 273}, {274, 196}, {215, 275}, 
{109, 276}, {277, 278}, {117, 113}, {228, 279}, 
{ 84, 280}, {123, 188}, {190, 235}, {281,  42}, 
{282, 236}, {283, 230}, {284,  87}, {132,  55}, 
{285,  38}, {125, 286}, {183, 140}, {287,  51}, 
{ 36, 288}, {206, 130}, { 35,  71}, {289,  18}, 
{ 29, 222}, { 49, 290}, { 25,  22}, { 66, 291}, 
{ 72,  79}, {198, 104}, {244, 292}, { 67, 208}, 
{ 39,  40}, {293, 102}, {245, 294}, {175, 220}, 
{237, 200}, { 27, 295}, {246, 214}, { 19, 296}, 
{ 33, 297}, {216,  23}, {298, 134}, {299, 204}, 
{103, 300}, {301, 302}, { 34,  65}, {303, 304}, 
{232, 305}, {176, 306}, { 26, 307}, { 47, 308}, 
{219, 309}, { 11, 310}, {108, 311}, {187,  20}, 
{119, 312}, {242, 136}, {144, 313}, { 68, 314}, 
{315,  88}, {316, 114}, {317,  99}, {318, 250}, 
{319, 221}, { 95, 320}, {321, 322}, {323, 324}, 
{ 60, 115}, {325,  80}, {326,  97}, {327, 328}, 
{110, 124}, {329, 330}, {116, 331}, {105, 332}, 
{195,  30}, {333, 334}, {  9, 335}, { 17, 336}, 
{126, 120}, { 62, 337}, {227, 338}, {339, 340}, 
{341, 152}, {342, 343}, {238, 101}, {344, 345}, 
{160, 346}, {347, 131}, {348, 349}, {350, 129}, 
{351, 122}, {225, 199}, {352, 193}, {353,  56}, 
{354, 355}, {356, 170}, {357,  28}, {358, 359}, 
{360, 361}, {362, 363}, {364, 241}, {365, 366}, 
{367, 368}, {369, 370}, {371,  13}, {  5, 372}, 
{111,  14}, { 10, 373}, {243, 374}, {143, 375}, 
{376,  85}, {377, 207}, {378, 379}, {380, 381}, 
{382, 383}, {384, 385}, {386,  12}, {387, 231}, 
{388, 159}, {389, 390}, {391, 392}, {393, 394}, 
{ 24, 395}, { 48, 396}, {397,   6}, {398, 399}, 
{400, 247}, {112, 401}, {402,  96}, {403, 404}, 
{405, 406}, {239, 407}, {408, 251}, {409, 191}, 
{410, 411}, {223, 412}, {413, 414}, {249, 415}, 
{416, 417}, {253, 418}, {419, 420}, {421, 422}, 
{135,  16}, {423,   8}, {424, 425}, {426,   4}, 
{427, 428}, {429, 430}, {431, 432}, {433, 434}, 
{435, 436}, {437, 438}, {439,   2}, { 64, 440}, 
{441, 442}, {443, 444}, {445, 446}, {447, 448}, 
{449, 450}, {240, 451}, {248, 452}, { 32, 453}, 
{454,   7}, {252, 455}, { 15, 456}, {224, 457}, 
{  3,  63}, {458,  31}, {459, 460}, {127, 461}, 
{192, 462}, {463, 254}, {464, 465}, {466, 467}, 
{468, 469}, {470, 471}, {128, 472}, {  1, 473}, 
{474, 475}, {476, 477}, {478, 479}, {480, 481}, 
{482, 483}, {484, 485}, {486, 487}, {488, 489}, 
{490, 491}, {492, 493}, {494, 495}, {496, 497}, 
{498, 499}, {500, 501}, {502, 503}, {504, 505}, 
{506, 255}, {507,   0}, {508, 509}, {  0,   0}, 
};

int32_t C4_AudioChunksPos[104] = {
         0,         29,        203,        328, 
       398,        444,        509,        623, 
       687,        700,        734,        792, 
       863,        948,        982,        991, 
      1000,       1066,       1096,       1283, 
      1312,       1364,       1444,       1490, 
      1536,       1545,       1577,       1737, 
      1842,       1933,       2050,       2155, 
      2244,       2344,       2360,       2401, 
      2595,       2731,       2821,       2889, 
      2978,       3104,       3187,       3218, 
      3328,       3448,       3539,       3604, 
      3656,       3677,       3697,       3746, 
      3785,       3991,       4033,       4099, 
      4199,       4300,       4357,       4381, 
      4433,       4453,       4476,       4517, 
      4707,       4845,       4950,       5063, 
      6090,       6090,       6090,       6090, 
      6090,       6090,       6090,       6090, 
      6090,       6090,       6090,       6090, 
      6090,       6090,       6090,       6090, 
      6090,       6090,       6090,       6090, 
      6090,       6090,       6090,       6090, 
      6090,       6090,       6090,       6090, 
      6090,       6090,       6090,       6090, 
      6090,       6090,       5099,       6904, 
};
int32_t C4_AudioChunksSize[104] = {
        29,        174,        125,         70, 
        46,         65,        114,         64, 
        13,         34,         58,         71, 
        85,         34,          9,          9, 
        66,         30,        187,         29, 
        52,         80,         46,         46, 
         9,         32,        160,        105, 
        91,        117,        105,         89, 
       100,         16,         41,        194, 
       136,         90,         68,         89, 
       126,         83,         31,        110, 
       120,         91,         65,         52, 
        21,         20,         49,         39, 
       206,         42,         66,        100, 
       101,         57,         24,         52, 
        20,         23,         41,        190, 
       138,        105,        113,       1027, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,       -991,       1805,          0, 
};
int32_t C4_GrChunksPos[627] = {
         0,        719,        727,        744, 
      2007,      13128,      19644,      20631, 
     21962,      24105,      26280,      28285, 
     30556,      32992,      34950,      37082, 
     38959,      39600,      39613,      40364, 
     41127,      41879,      42546,      43173, 
     43576,      43635,      43690,      43741, 
     43793,      43845,      43858,      44754, 
     45647,      46540,      47458,      48346, 
     49152,      49996,      50982,      52024, 
     52714,      53465,      54177,      54866, 
     55564,      56235,      56997,      57678, 
     58283,      58907,      59584,      59896, 
     60383,      61170,      62088,      63215, 
     64071,      64929,      65748,      66614, 
     67659,      68864,      70167,      71110, 
     72090,      72854,      73333,      74489, 
     75703,      76905,      78162,      79047, 
     79936,      80829,      81624,      82286, 
     83061,      83689,      84213,      84499, 
     84772,      85305,      85919,      86624, 
     87428,      88204,      88984,      89770, 
     90534,      91243,      91983,      92898, 
     93610,      94190,      94791,      95386, 
     95982,      96503,      97055,      97615, 
     98105,      98534,      98940,      99739, 
    100509,     101274,     102064,     102876, 
    103862,     104882,     105833,     106718, 
    107408,     108047,     108345,     108642, 
    108936,     109232,     109534,     109828, 
    110691,     111116,     111541,     112185, 
    112857,     113222,     113480,     113749, 
    114007,     114275,     114496,     114719, 
    114941,     115165,     115388,     115612, 
    115845,     116077,     116306,     116536, 
    116868,     117600,     117923,     118096, 
    118268,     118651,     119143,     119598, 
    120667,     121680,     122733,     123748, 
    124872,     126135,     127221,     128308, 
    129535,     130449,     131142,     132028, 
    132904,     133790,     134664,     135632, 
    136698,     137628,     138580,     139609, 
    140417,     141107,     141905,     142696, 
    143202,     143834,     144258,     144672, 
    145089,     145539,     145958,     146380, 
    146790,     147145,     148219,     149293, 
    150041,     150810,     151500,     152045, 
    152611,     153197,     153758,     154236, 
    154572,     154588,     155411,     156340, 
    157314,     158268,     159292,     160277, 
    161684,     162990,     164402,     165707, 
    166872,     167863,     168779,     170022, 
    171107,     172429,     173637,     174854, 
    175889,     177335,     178692,     180090, 
    181164,     182419,     183467,     184677, 
    186266,     188109,     189393,     190746, 
    192414,     194345,     195841,     196982, 
    198083,     199325,     200540,     202009, 
    203416,     204932,     206403,     207766, 
    208905,     209804,     210620,     211966, 
    213229,     214572,     215733,     217177, 
    218559,     220191,     221558,     222964, 
    224326,     225695,     226861,     227952, 
    228899,     229960,     230890,     232068, 
    233074,     234594,     236022,     236996, 
    237880,     238878,     239765,     241258, 
    242471,     243566,     244791,     245890, 
    247493,     249104,     250580,     252077, 
    253470,     254725,     256090,     257376, 
    258726,     260139,     261503,     262723, 
    263941,     265069,     266137,     267522, 
    268716,     269679,     270467,     271423, 
    272210,     273103,     274240,     275402, 
    276575,     277689,     278800,     280042, 
    281037,     282259,     283247,     284389, 
    285375,     286482,     287654,     288730, 
    289906,     290987,     292174,     293300, 
    294374,     295656,     296791,     297875, 
    298910,     298930,     300553,     300585, 
    302089,     302414,     302447,     302574, 
    302696,     302803,     302916,     303014, 
    303089,     303189,     303289,     303388, 
    303490,     303590,     303708,     303785, 
    303862,     303938,     304014,     304092, 
    304178,     304282,     304353,     304452, 
    304526,     304626,     304733,     304839, 
    304954,     305060,     305188,     305275, 
    305364,     305441,     305563,     305684, 
    305808,     305924,     306027,     306158, 
    306255,     306344,     306436,     306549, 
    306673,     306795,     306924,     307009, 
    307105,     307223,     307330,     307432, 
    307505,     307606,     307708,     307828, 
    307932,     308049,     308161,     308234, 
    308311,     308383,     308499,     308597, 
    308699,     308803,     308908,     309016, 
    309127,     309233,     309326,         -1, 
        -1,         -1,         -1,         -1, 
    309431,         -1,         -1,         -1, 
        -1,         -1,         -1,         -1, 
        -1,         -1,         -1,         -1, 
        -1,         -1,         -1,         -1, 
        -1,         -1,         -1,         -1, 
        -1,         -1,         -1,         -1, 
        -1,         -1,         -1,         -1, 
        -1,         -1,         -1,         -1, 
        -1,         -1,         -1,         -1, 
        -1,         -1,         -1,         -1, 
        -1,         -1,         -1,         -1, 
        -1,         -1,         -1,         -1, 
        -1,         -1,         -1,         -1, 
        -1,         -1,         -1,         -1, 
        -1,         -1,         -1,         -1, 
        -1,         -1,         -1,         -1, 
        -1,         -1,         -1,         -1, 
        -1,         -1,         -1,         -1, 
        -1,         -1,         -1,         -1, 
        -1,         -1,         -1,         -1, 
        -1,         -1,         -1,         -1, 
        -1,         -1,         -1,         -1, 
        -1,         -1,         -1,         -1, 
        -1,         -1,         -1,         -1, 
        -1,         -1,         -1,         -1, 
        -1,         -1,         -1,         -1, 
        -1,     309496,     309551,     309633, 
    309716,     309791,     309879,     309953, 
    310027,     310107,     310198,     310279, 
    310354,     310443,     310515,     310606, 
    310697,     310787,     310867,     310952, 
    311033,     311107,     311175,     311259, 
    311342,     311433,     311522,     311599, 
    311672,     311727,     311782,     311837, 
    311892,     311947,     312002,     312057, 
        -1,         -1,     312116,     312254, 
    312376,     312512,     312643,     312757, 
    312871,     312982,     313084,     313190, 
    313289,     313394,     313532,     313665, 
    313801,     313937,     314068,     314205, 
    314335,     314473,     314570,     314672, 
    314779,     314878,     314982,     315040, 
    315154,     315242,     315364,     315484, 
    315589,     315727,     315861,     315993, 
    316130,     316262,     316373,     316477, 
    316549,     316670,     316769,     316880, 
    316984,     317056,     317177,     317276, 
    317393,     317516,     317629,     317720, 
    317821,     317922,     318030,     318142, 
    318242,     318355,     318476,     318565, 
    318685,     318802,     318922,     319053, 
    319182,     319292,     319404,     319521, 
    319637,     319760,     319883,     319989, 
    320107,     320224,     320890,     321489, 
    322061,     322694,     323329,     323779, 
    324349,     324950,     325538,     326086, 
    326688,     327278,     327833,     328487, 
    329099,     329562,     330385,     330865, 
    331349,     331906,     335994, };
int32_t C4_GrChunksSize[627] = {
       719,          8,         17,       1263, 
     11121,       6516,        987,       1331, 
      2143,       2175,       2005,       2271, 
      2436,       1958,       2132,       1877, 
       641,         13,        751,        763, 
       752,        667,        627,        403, 
        59,         55,         51,         52, 
        52,         13,        896,        893, 
       893,        918,        888,        806, 
       844,        986,       1042,        690, 
       751,        712,        689,        698, 
       671,        762,        681,        605, 
       624,        677,        312,        487, 
       787,        918,       1127,        856, 
       858,        819,        866,       1045, 
      1205,       1303,        943,        980, 
       764,        479,       1156,       1214, 
      1202,       1257,        885,        889, 
       893,        795,        662,        775, 
       628,        524,        286,        273, 
       533,        614,        705,        804, 
       776,        780,        786,        764, 
       709,        740,        915,        712, 
       580,        601,        595,        596, 
       521,        552,        560,        490, 
       429,        406,        799,        770, 
       765,        790,        812,        986, 
      1020,        951,        885,        690, 
       639,        298,        297,        294, 
       296,        302,        294,        863, 
       425,        425,        644,        672, 
       365,        258,        269,        258, 
       268,        221,        223,        222, 
       224,        223,        224,        233, 
       232,        229,        230,        332, 
       732,        323,        173,        172, 
       383,        492,        455,       1069, 
      1013,       1053,       1015,       1124, 
      1263,       1086,       1087,       1227, 
       914,        693,        886,        876, 
       886,        874,        968,       1066, 
       930,        952,       1029,        808, 
       690,        798,        791,        506, 
       632,        424,        414,        417, 
       450,        419,        422,        410, 
       355,       1074,       1074,        748, 
       769,        690,        545,        566, 
       586,        561,        478,        336, 
        16,        823,        929,        974, 
       954,       1024,        985,       1407, 
      1306,       1412,       1305,       1165, 
       991,        916,       1243,       1085, 
      1322,       1208,       1217,       1035, 
      1446,       1357,       1398,       1074, 
      1255,       1048,       1210,       1589, 
      1843,       1284,       1353,       1668, 
      1931,       1496,       1141,       1101, 
      1242,       1215,       1469,       1407, 
      1516,       1471,       1363,       1139, 
       899,        816,       1346,       1263, 
      1343,       1161,       1444,       1382, 
      1632,       1367,       1406,       1362, 
      1369,       1166,       1091,        947, 
      1061,        930,       1178,       1006, 
      1520,       1428,        974,        884, 
       998,        887,       1493,       1213, 
      1095,       1225,       1099,       1603, 
      1611,       1476,       1497,       1393, 
      1255,       1365,       1286,       1350, 
      1413,       1364,       1220,       1218, 
      1128,       1068,       1385,       1194, 
       963,        788,        956,        787, 
       893,       1137,       1162,       1173, 
      1114,       1111,       1242,        995, 
      1222,        988,       1142,        986, 
      1107,       1172,       1076,       1176, 
      1081,       1187,       1126,       1074, 
      1282,       1135,       1084,       1035, 
        20,       1623,         32,       1504, 
       325,         33,        127,        122, 
       107,        113,         98,         75, 
       100,        100,         99,        102, 
       100,        118,         77,         77, 
        76,         76,         78,         86, 
       104,         71,         99,         74, 
       100,        107,        106,        115, 
       106,        128,         87,         89, 
        77,        122,        121,        124, 
       116,        103,        131,         97, 
        89,         92,        113,        124, 
       122,        129,         85,         96, 
       118,        107,        102,         73, 
       101,        102,        120,        104, 
       117,        112,         73,         77, 
        72,        116,         98,        102, 
       104,        105,        108,        111, 
       106,         93,        105,          0, 
         0,          0,          0,          0, 
        65,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,         55,         82,         83, 
        75,         88,         74,         74, 
        80,         91,         81,         75, 
        89,         72,         91,         91, 
        90,         80,         85,         81, 
        74,         68,         84,         83, 
        91,         89,         77,         73, 
        55,         55,         55,         55, 
        55,         55,         55,         59, 
         0,          0,        138,        122, 
       136,        131,        114,        114, 
       111,        102,        106,         99, 
       105,        138,        133,        136, 
       136,        131,        137,        130, 
       138,         97,        102,        107, 
        99,        104,         58,        114, 
        88,        122,        120,        105, 
       138,        134,        132,        137, 
       132,        111,        104,         72, 
       121,         99,        111,        104, 
        72,        121,         99,        117, 
       123,        113,         91,        101, 
       101,        108,        112,        100, 
       113,        121,         89,        120, 
       117,        120,        131,        129, 
       110,        112,        117,        116, 
       123,        123,        106,        118, 
       117,        666,        599,        572, 
       633,        635,        450,        570, 
       601,        588,        548,        602, 
       590,        555,        654,        612, 
       463,        823,        480,        484, 
       557,       4088,          0, };
uint16_t C4_maphead_RLEWtag = -21555;
int32_t C4_maphead_headeroffsets[100] = {
      1048,       2036,       3056,       4040, 
      5000,       6034,       7128,       8300, 
      9260,      10669,      11880,      12961, 
     13595,      14813,      15703,      16108, 
     16759,      17942,      18935,      19144, 
     19372,      19648,      20391,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
         0,          0,          0,          0, 
};
uint8_t C4_tile_flags[648] = {
0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
0x01, 0x01, 0x07, 0x07, 0x03, 0x03, 0x03, 0x03, 
0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01, 0x01, 
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
0x01, 0x01, 0x01, 0x03, 0x01, 0x01, 0x03, 0x01, 
0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};


void SPD_ExtractC4Data() {
	char name[256];
	long size;
	uint8_t *buf;

	buf = SPD_ParseObj("ABSADICT.OBJ", name, &size);
	SPD_DumpDict("C4_audiohuffman", buf, size);
	free(buf);
	buf = SPD_ParseObj("ABSEDICT.OBJ", name, &size);
	SPD_DumpDict("C4_grhuffman", buf, size);
	free(buf);

	buf = SPD_ParseObj("ABSAHEAD.OBJ", name, &size);
	SPD_DumpLongArray("C4_Audio", buf, size, 4);
	free(buf);
	buf = SPD_ParseObj("ABSEHEAD.OBJ", name, &size);
	SPD_DumpLongArray("C4_Gr", buf, size, 3);
	free(buf);
	buf = SPD_ParseObj("ABSMHEAD.OBJ", name, &size);
	SPD_DumpMapfiletype("C4_", buf, size);
	
	assert(size > 402+216);
	printf("uint8_t C4_tile_flags[%i] = {\n", size-402+216);
	SPD_DumpU8Hex(buf+402+216, size-402-216);
	printf("};\n");	
	free(buf);
}

void SPD_DumpHexChar(const uint8_t *Buffer, long Size);

void loadAdlibSounds(uint8_t *Data, huffnode *HuffmanTable) {
	SPA_InitSamples(NUMSOUNDS, 1);

	int i;
	for (i=0; i < NUMSOUNDS; i++) {
		printf("Audio chunk %i\n", STARTADLIBSOUNDS+i);
		SPD_DumpHexChar(Data+AudioChunksPos[STARTADLIBSOUNDS+i], AudioChunksSize[STARTADLIBSOUNDS+i]);
		long len;
		memptr buffer = SPD_ReadAndHuffExpand(Data+AudioChunksPos[STARTADLIBSOUNDS+i], HuffmanTable, &len);
		SoundCommon *s =  buffer;
		printf("decompressed:\n");
		SPD_DumpHexChar(buffer, len);
		SPA_RenderSample(i, buffer);
		free(buffer);
	}
	{
		int i=0;
		memptr buffer = SPD_ReadAndHuffExpand(Data+AudioChunksPos[STARTMUSIC+i], HuffmanTable, NULL);
		SPA_RenderMusic(i, buffer);
		free(buffer);
	}
}

static void loadMapHeaders(uint8_t *Data) {
extern maptype mapheaderseg[30];
	memset(&mapheaderseg, 0, sizeof(mapheaderseg));
	int i;
	for (i = TOWNE_CEMETARY_MAP; i < LASTMAP; i++) {
		loadMapHeader(Data,C4_maphead_headeroffsets[i], i, C4_maphead_RLEWtag);
	}	
}

void SPD_SetupGameData() {
SPD_ExtractC4Data();

	grhuffman = C4_grhuffman;
	GrChunksPos = C4_GrChunksPos;
	GrChunksSize = C4_GrChunksSize;
	AudioChunksPos = C4_AudioChunksPos;
	AudioChunksSize = C4_AudioChunksSize;
	mapData = SPD_ReadFile("GAMEMAPS.ABS", &mapDataSize);
	auData = SPD_ReadFile("AUDIO.ABS", &auDataSize);
	grData = SPD_ReadFile("EGAGRAPH.ABS", &grDataSize);

	grsegs = malloc(sizeof(void*)*NUMCHUNKS);
	memset(grsegs, 0, sizeof(void*)*NUMCHUNKS);

	loadMapHeaders(mapData);
	loadAdlibSounds(auData, C4_audiohuffman);

	pictable = loadPicTable(grData+GrChunksPos[STRUCTPIC], GrChunksSize[STRUCTPIC], NUMPICS);
	picmtable = loadPicTable(grData+GrChunksPos[STRUCTPICM], GrChunksSize[STRUCTPICM], NUMPICM);
	spritetable = loadSpriteTable(grData+GrChunksPos[STRUCTSPRITE], GrChunksSize[STRUCTSPRITE], NUMSPRITES);
}

void SPD_CombinedLoader(int Chunk) {
	if (Chunk >= STARTFONT && Chunk < STARTFONT+NUMFONT) {
		loadFont(Chunk);
	} else if (Chunk >= STARTPICS && Chunk < STARTPICS+NUMPICS) {
		loadPic(Chunk, Chunk-STARTPICS);
	} else if (Chunk >= STARTPICM && Chunk < STARTPICM+NUMPICM) {
		loadMaskedPic(Chunk, Chunk-STARTPICM);
	} else if (Chunk >= STARTSPRITES && Chunk < STARTSPRITES+NUMSPRITES) {
		loadSprite(Chunk, Chunk-STARTSPRITES);
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


