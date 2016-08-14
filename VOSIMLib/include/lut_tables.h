#ifndef __LUT_TABLES__
#define __LUT_TABLES__
/*::macro_defs::*/
/*::/macro_defs::*/

/*::table_decl::*/
extern double BLIMP_TABLE_OFFLINE[];
extern double BLIMP_TABLE_ONLINE[];
extern double PITCH_TABLE[];
extern double BL_SAW_TABLE[];
extern double BL_SQUARE_TABLE[];
extern double BL_TRI_TABLE[];
extern double SIN_TABLE[];
/*::/table_decl::*/

namespace syn {
	class LookupTable;
	class ResampledLookupTable;
	class BlimpTable;

	/*::lut_decl::*/
	BlimpTable& lut_blimp_table_offline();
	BlimpTable& lut_blimp_table_online();
	LookupTable& lut_pitch_table();
	ResampledLookupTable& lut_bl_saw_table();
	ResampledLookupTable& lut_bl_square_table();
	ResampledLookupTable& lut_bl_tri_table();
	LookupTable& lut_sin_table();
	/*::/lut_decl::*/
}
#endif