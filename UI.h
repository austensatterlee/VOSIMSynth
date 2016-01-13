#ifndef __UI__
#define __UI__
#include "NDPoint.h"
#include <vector>
#define SCREEN_PAD_L 25
#define X_PAD 10
#define Y_PAD 32
#define CELL_SIZE 32

using namespace std;
namespace syn
{
	class ColorPoint : public NDPoint<3>
	{
	public:
		ColorPoint(unsigned int word) :
			NDPoint<3>(double((word >> 16) & 0xFF), double((word >> 8) & 0xFF), double(word & 0xFF))
		{}
		ColorPoint(const NDPoint<3>& pt) :
			NDPoint<3>(pt)
		{
		}
		operator IColor() const
		{
			return IColor(255,int(m_pvec[0]), int(m_pvec[1]), int(m_pvec[2]));
		}
	};

	enum PALETTE
	{
		PRIMARY = 0,
		SECONDARY,
		TERTIARY,
		COMPLEMENT
	};

	const ColorPoint globalPalette[4][5] = {
	  { 0x646900,
		0xF1FD00,
		0xA8B005,
		0x272900,
		0x0A0A00 },
	  { 0x071848,
		0x1041C9,
		0x102C78,
		0x02091C,
		0x010207 },
	  { 0x6A4800,
		0xFFAD00,
		0xB27B05,
		0x291C00,
		0x0A0700 },
	  { 0x330347,
		0x9106C8,
		0x570876,
		0x14011B,
		0x050007 }
	};
}
#endif