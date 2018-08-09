#include "vosimlib/tables.h"
#include "vosimlib/lut_tables.h"

namespace syn {
    /*::table_decl::*/
    extern double BLIMP_TABLE_OFFLINE[];
    extern double BLIMP_TABLE_ONLINE[];
    extern double PITCH_TABLE[];
    extern double BL_SAW_TABLE[];
    extern double BL_SQUARE_TABLE[];
    extern double BL_TRI_TABLE[];
    extern double SIN_TABLE[];
    /*::/table_decl::*/

    /*::lut_defs::*/
    BlimpTable& lut_blimp_table_offline() { static BlimpTable table(BLIMP_TABLE_OFFLINE, 204801, 200, 2048); return table; }
    BlimpTable& lut_blimp_table_online() { static BlimpTable table(BLIMP_TABLE_ONLINE, 20481, 20, 2048); return table; }
    AffineTable& lut_pitch_table() { static AffineTable table(PITCH_TABLE, 3841, -128.0, 256.0); return table; }
    ResampledTable& lut_bl_saw_table() { static ResampledTable table(BL_SAW_TABLE, 2048, lut_blimp_table_online(), lut_blimp_table_offline()); return table; }
    ResampledTable& lut_bl_square_table() { static ResampledTable table(BL_SQUARE_TABLE, 2048, lut_blimp_table_online(), lut_blimp_table_offline()); return table; }
    ResampledTable& lut_bl_tri_table() { static ResampledTable table(BL_TRI_TABLE, 2048, lut_blimp_table_online(), lut_blimp_table_offline()); return table; }
    NormalTable& lut_sin_table() { static NormalTable table(SIN_TABLE, 1024); return table; }
    /*::/lut_defs::*/
}
