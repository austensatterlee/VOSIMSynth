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
			NDPoint<3>(double((word >> 16) & 0xFF), double((word >> 8) & 0xFF), double(word & 0xFF)) {}

		ColorPoint(const NDPoint<3>& pt) :
			NDPoint<3>(pt) { }

		operator IColor() const {
			return IColor(255, int(m_pvec[0]), int(m_pvec[1]), int(m_pvec[2]));
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
		{
			0x3F51B5,
			0x5C6BC0,
			0x7986CB,
			0x9FA8DA,
			0xC5CAE9,
		},
		{
			0x2196F3,
			0x42A5F5,
			0x64B5F6,
			0x90CAF9,
			0xBBDEFB
		},
		{
			0x4CAF50,
			0x66BB6A,
			0x81C784,
			0xA5D6A7,
			0xC8E6C9
		},
		{
			0x80D8FF,
			0x40C4FF,
			0x00B0FF,
			0x0091EA,
			0x01579B
		}
	};
}
#endif

