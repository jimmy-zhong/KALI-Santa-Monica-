#include "biquads.h"

const EQ_STRUCT PresetEQ[8][8] = {
{			// Preset No. 1
//{En, Type,  Freq, Qfx100,  Gain}
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
},

{			// Preset No. 2
//{En, Type,  Freq, Qfx100,  Gain}
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
},

{			// Preset No. 3
//{En, Type,  Freq, Qfx100,  Gain}
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
},

{			// Preset No. 4
//{En, Type,  Freq, Qfx100,  Gain}
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
},

{			// Preset No. 5
//{En, Type,  Freq, Qfx100,  Gain}
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
},

{			// Preset No. 6
//{En, Type,  Freq, Qfx100,  Gain}
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
},

{			// Preset No. 7
//{En, Type,  Freq, Qfx100,  Gain}
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
},

{			// Preset No. 8
//{En, Type,  Freq, Qfx100,  Gain}
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
{   0,    4,  1000,    141, -1000},
},
};



