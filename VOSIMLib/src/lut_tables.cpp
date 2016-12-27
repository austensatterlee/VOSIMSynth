#include "lut_tables.h"
#include "tables.h"
namespace syn {
    /*::lut_defs::*/
    BlimpTable& lut_blimp_table_offline() { static BlimpTable table(BLIMP_TABLE_OFFLINE, 263169, 257, 2048); return table; }
    BlimpTable& lut_blimp_table_online() { static BlimpTable table(BLIMP_TABLE_ONLINE, 11265, 11, 2048); return table; }
    LookupTable& lut_pitch_table() { static LookupTable table(PITCH_TABLE, 1024, -128, 128, false); return table; }
    ResampledLookupTable& lut_bl_saw_table() { static ResampledLookupTable table(BL_SAW_TABLE, 8193, lut_blimp_table_online(), lut_blimp_table_offline()); return table; }
    ResampledLookupTable& lut_bl_square_table() { static ResampledLookupTable table(BL_SQUARE_TABLE, 8193, lut_blimp_table_online(), lut_blimp_table_offline()); return table; }
    ResampledLookupTable& lut_bl_tri_table() { static ResampledLookupTable table(BL_TRI_TABLE, 8193, lut_blimp_table_online(), lut_blimp_table_offline()); return table; }
    LookupTable& lut_sin_table() { static LookupTable table(SIN_TABLE, 1024, 0, 1, true); return table; }
    /*::/lut_defs::*/
}