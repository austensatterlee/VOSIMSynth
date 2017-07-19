#pragma once
#include "vosimlib/common.h"

/*::macro_defs::*/
/*::/macro_defs::*/

namespace syn{
    /*::table_decl::*/
    extern double BLIMP_TABLE_OFFLINE[];
    extern double BLIMP_TABLE_ONLINE[];
    extern double PITCH_TABLE[];
    extern double BL_SAW_TABLE[];
    extern double BL_SQUARE_TABLE[];
    extern double BL_TRI_TABLE[];
    extern double SIN_TABLE[];
    /*::/table_decl::*/

    /*::lut_decl::*/
    class AffineTable;
    class ResampledTable;
    class NormalTable;
    class BlimpTable;
    BlimpTable& VOSIMLIB_API lut_blimp_table_offline();
    BlimpTable& VOSIMLIB_API lut_blimp_table_online();
    AffineTable& VOSIMLIB_API lut_pitch_table();
    ResampledTable& VOSIMLIB_API lut_bl_saw_table();
    ResampledTable& VOSIMLIB_API lut_bl_square_table();
    ResampledTable& VOSIMLIB_API lut_bl_tri_table();
    NormalTable& VOSIMLIB_API lut_sin_table();
    /*::/lut_decl::*/
}
