# Names dict
names = {
    'DIEGO': 10,
    'BUBA': 12,
    'MOE': 13,
    'LUIS': 14,
    'D': 10,
    'B': 12,
    'M': 13,
    'L': 14,
    'ALL': 99
}

'''
{
    State {
        Frequency band# : [list of channels]
    }
}
'''
channels_freq = {
    1: {
        1: [50, 59, 68, 77, 86, 95, 104, 113, 122],
        2: [30, 31, 32, 33, 34, 35]
    },
    2: {
        1: [59, 68, 77, 104, 113],
        2: [46, 45]
    },
    3: {
        1: [50, 59, 68, 77, 86, 95, 104, 113, 122],
        2: [46, 45]
    },
    4: {
        1: [68, 104, 113]
    },
    5: {
        1: [50, 59, 68, 77, 86, 95, 104, 113, 122],
        2: [30, 31, 32, 33, 34, 35]
    },
    6: {
        1: [50, 59, 68, 77, 86, 95, 104, 113, 122]
    },
}

'''
{
    State {
        Frequency band #: [min intensity, max intensity]
    }
}
'''
ranges_freq = {
    1: {
        1: [15, 30],
        2: [20, 50]
    },
    2: {
        1: [15, 30],
        2: [20, 50]
    },
    3: {
        1: [15, 30],
        2: [20, 50]
    },
    4: {
        1: [15, 30],
        2: [20, 50]
    },
    5: {
        1: [40, 100],
        2: [30, 100]
    },
    6: {
        1: [50, 100]
    },
}

personality = {
    'RED': 1,
    'GREEN': 2,
    'BLUE': 3,
    'WHITE': 4,
    'AMBER': 5,
    'INDIGO': 6,
    'STROBE': 7,
}

patch = {
	1: 92,
    2: 94,
    3: 88,
    7: 90,
    8: 95,
    9: 72,
    10: 82,
    11: 5,
    12: 16,
    13: 9,
    14: 20,
    15: 24,
    16: 96,
    17: 71,
    18: 169,
    19: 170,
    20: 227,
    21: 221,
    22: 50,
    23: 49,
    24: 51,
    25: 78,
    26: 93,
    27: 91,
    28: 90,
    29: 89,
    30: 164,
    31: 226,
    32: 224,
    33: 158,
    34: 220,
    35: 218,
    36: 163,
    37: 225,
    38: 223,
    39: 157,
    40: 219,
    41: 217,
    42: 166,
    43: 165,
    44: 228,
    45: 160,
    46: 159,
    47: 222,
    50: 370,
    51: 371,
    52: 372,
    53: 373,
    54: 374,
    55: 375,
    56: 376,
    57: 377,
    58: 378,
    59: 379,
    60: 380,
    61: 381,
    62: 382,
    63: 383,
    64: 384,
    65: 385,
    66: 386,
    67: 387,
    68: 388,
    69: 389,
    70: 390,
    71: 391,
    72: 392,
    73: 393,
    74: 394,
    75: 395,
    76: 396,
    77: 397,
    78: 398,
    79: 399,
    80: 400,
    81: 401,
    82: 402,
    83: 403,
    84: 404,
    85: 405,
    86: 406,
    87: 407,
    88: 408,
    89: 409,
    90: 410,
    91: 411,
    92: 412,
    93: 413,
    94: 414,
    95: 415,
    96: 416,
    97: 417,
    98: 418,
    99: 419,
    100: 420,
    101: 421,
    102: 422,
    103: 423,
    104: 424,
    105: 425,
    106: 426,
    107: 427,
    108: 428,
    109: 429,
    110: 430,
    111: 431,
    112: 432,
    113: 433,
    114: 434,
    115: 435,
    116: 436,
    117: 437,
    118: 438,
    119: 439,
    120: 440,
    121: 441,
    122: 442,
    123: 443,
    124: 444,
    125: 445,
    126: 446,
    127: 447,
    128: 448,
    130: 449,
    131: 450,
    133: 133,
    134: 134,
    135: 135,
    136: 136,
    137: 137,
    138: 138,
    139: 139,
    140: 140,
    141: 141,
    142: 142,
    143: 143,
    144: 144,
    200: 66,
    201: 79,
    202: 82,
    203: 84,
    204: 90,
    205: 91,
    206: 93,
    500: 500,
    509: 509,
    510: 510,
    511: 511,
    489: 489,
    490: 490,
    491: 491,
    492: 492,
    493: 493,
    494: 494,
    495: 495,
    496: 496,
    497: 497,
    498: 498,
    499: 499,
}
