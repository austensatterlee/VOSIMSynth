/*
Copyright 2016, Austen Satterlee

This file is part of VOSIMProject.

VOSIMProject is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

VOSIMProject is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with VOSIMProject. If not, see <http://www.gnu.org/licenses/>.
*/

/**  
 *	 \file KeyboardUnit.h
 *   \brief 
 *   \details
 *   \author Austen Satterlee
 *   \date 03/27/2016
 */
#ifndef __KEYBOARDCONTROL__
#include "VOSIMSynth.h"
#include "UI.h"
#include "IControl.h"


namespace syn
{
	class KeyboardControl : public IControl
	{
	private:
		int m_numKeys, m_firstKey;
		bool m_autoNumKeys;
		vector<NDPoint<2, int>> m_keyPosVec;
		NDPoint<2, int> m_whtKeySize, m_blkKeySize;
		vector<int> m_isPressed;
		int m_lastPressed;
		unsigned m_drawCnt;
		shared_ptr<VoiceManager> m_vm;
		CachedImage m_adgKey, m_beKey, m_cfKey;
		CachedImage m_blkKey;
		IMouseMod m_lastMouseMod;
	protected:
		void loadKeyBmp_(LICE_IBitmap* a_dst, int a_RESID, IGraphics* a_graphics) const {
			// ReSharper restore CppMemberFunctionMayBeConst
			IBitmap bmp = a_graphics->LoadIBitmap(a_RESID, ".png");
			a_dst->resize(bmp.W, bmp.H);
			LICE_Blit(a_dst, static_cast<LICE_IBitmap*>(bmp.mData), 0, 0, 0, 0, bmp.W, bmp.H, 1.0, LICE_BLIT_USE_ALPHA | LICE_BLIT_MODE_COPY);
		}

		void sendNoteOn_(int a_keyidx, int a_velocity) {
			if (m_isPressed[a_keyidx]>=0) {
				//IMidiMsg msg;
				//msg.MakeNoteOnMsg(m_firstKey + a_keyidx, a_velocity, 0);
				//mPlug->ProcessMidiMsg(&msg);
				m_isPressed[a_keyidx]=1;
				m_vm->noteOn(m_firstKey + a_keyidx, a_velocity);
				m_lastPressed = a_keyidx;
			}
		}

		void sendNoteOff_(int a_keyidx) {
			if (m_isPressed[a_keyidx] > 0) {
				//IMidiMsg msg;
				//msg.MakeNoteOffMsg(m_firstKey + a_keyidx, 0);
				//mPlug->ProcessMidiMsg(&msg);
				m_isPressed[a_keyidx]=0;
				m_vm->noteOff(m_firstKey + a_keyidx, 0);
			}
		}

		/**
		 * Determines which key (if any) the provided pixel lies on.
		 * For example, if the pixel lied on the first key, then 0 would be returned.
		 * If the pixel did not lie on any key, -1 is returned.
		 */
		int getPressedKey_(int a_x, int a_y) {
			NDPoint<2, int> keysize;
			for (int i = 0; i < m_numKeys; i++) {
				keysize = getKeySize(i);
				IRECT keyrect{m_keyPosVec[i][0],m_keyPosVec[i][1],int(m_keyPosVec[i][0] + keysize[0]),int(m_keyPosVec[i][1] + keysize[1])};
				if((i<m_numKeys-1) && (&getImageFromKeyIdx_(i+1)==&m_blkKey)) {
					int nxtkeyind = i + 1;
					NDPoint<2, int> blkkeysize = getKeySize(nxtkeyind);
					IRECT nextBlkKeyRect{ m_keyPosVec[nxtkeyind][0],m_keyPosVec[nxtkeyind][1],int(m_keyPosVec[nxtkeyind][0] + blkkeysize[0]),int(m_keyPosVec[nxtkeyind][1] + blkkeysize[1]) };
					if (nextBlkKeyRect.Contains(a_x, a_y)) {
						return nxtkeyind;
					}
				}

				if (keyrect.Contains(a_x, a_y)) {
					return i;
				}
			}
			return -1;
		}

		CachedImage& getImageFromKeyIdx_(int a_keyidx) {
			int semitone = (a_keyidx + m_firstKey) % 12;
			if (semitone == 9 || semitone == 2 || semitone == 7)
				return m_adgKey;
			if (semitone == 4 || semitone == 11)
				return m_beKey;
			if (semitone == 0 || semitone == 5)
				return m_cfKey;
			return m_blkKey;
		}

		NDPoint<2,int> getKeySize(int a_keyidx) const {
			int semitone = (a_keyidx + m_firstKey) % 12;
			if (semitone == 1 || semitone == 3 || semitone == 6 || semitone == 8 || semitone == 10) {
				return m_blkKeySize;
			}
			return m_whtKeySize;
		}

	public:
		KeyboardControl(IPlugBase* a_plug, shared_ptr<VoiceManager> a_vm, const IRECT& a_rect, int a_firstKey = 36, int a_numKeys=36)
			:
			IControl{a_plug, a_rect, -1},
			m_numKeys(a_numKeys),
			m_firstKey(a_firstKey),
			m_autoNumKeys(false),
			m_lastPressed(-1),
			m_drawCnt(0),
			m_vm(a_vm) 
		{
			if(a_numKeys<=0) {
				m_autoNumKeys = true;
			}
			m_adgKey.resetConds(&mRECT, &m_numKeys, &m_lastPressed);
			m_beKey.resetConds(&mRECT, &m_numKeys, &m_lastPressed);
			m_cfKey.resetConds(&mRECT, &m_numKeys, &m_lastPressed);
			m_blkKey.resetConds(&mRECT, &m_numKeys, &m_lastPressed);
			mDblAsSingleClick = true;
		}

		void setNumKeys(int a_numKeys) {			
			int newNumKeys;
			if (a_numKeys <= 0 || m_autoNumKeys) 
			{ // Determine the number of keys from the draw rectangle's width
				newNumKeys = (mRECT.W() * 12.0) / (m_adgKey.width() * 7.0);
				m_whtKeySize = m_adgKey.size();
				m_blkKeySize = m_blkKey.size();
			} 
			else 
			{ // Determine the key size from the draw rectangle's width and the requested number of keys
				newNumKeys = a_numKeys;
				m_whtKeySize = NDPoint<2, int>((mRECT.W() * 12.0) / (newNumKeys * 7.0), mRECT.H());
				m_blkKeySize = (NDPoint<2, double>(m_whtKeySize) / NDPoint<2,double>(m_adgKey.size())) * m_blkKey.size();
			}

			// Release keys that will be disappearing
			for (int i = newNumKeys; i < m_numKeys; i++) {
				if (m_isPressed[i]) {
					m_isPressed[i] = false;
					sendNoteOff_(i);
				}
			}
						
			m_numKeys = a_numKeys;
			m_keyPosVec.resize(a_numKeys);
			m_isPressed.resize(a_numKeys, false);
		}

		void OnMouseDown(int x, int y, IMouseMod* pMod) override {
			int pressed = getPressedKey_(x, y);
			if (pressed >= 0) {
				sendNoteOn_(pressed, 0xFF * INVLERP(mRECT.T, mRECT.B, 1.0*y));
			}
			m_lastMouseMod = *pMod;
		}

		void OnMouseUp(int x, int y, IMouseMod* pMod) override {
			int pressed = getPressedKey_(x, y);
			if (pressed >= 0 && !m_lastMouseMod.R) {
				sendNoteOff_(pressed);
			}
		}

		void OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod) override {
			int pressed = getPressedKey_(x, y);
			if (pressed >= 0 && pressed != m_lastPressed && m_lastPressed >= 0) {				
				if (m_lastPressed >= 0)
					sendNoteOff_(m_lastPressed);
				sendNoteOn_(pressed, 0xFF * INVLERP(mRECT.T, mRECT.B, 1.0*y));
			}
			m_lastMouseMod = *pMod;
		}

		bool IsDirty() override {
			return true;
		};

		bool Draw(IGraphics* pGraphics) override {
			LICE_SysBitmap* canvas = pGraphics->GetDrawBitmap();
			
			setNumKeys(m_numKeys);

			// Draw keyboard
			m_keyPosVec[0] = { mRECT.L,mRECT.T };
			int keyidx;
			NDPoint<2, int> keysize;
			for (keyidx = 0; keyidx < m_numKeys; keyidx++) {
				CachedImage& src = getImageFromKeyIdx_(keyidx);
				keysize = getKeySize(keyidx);
				// Adjust black key position so that it overlaps equally onto adjacent white keys
				if (&src == &m_blkKey && keyidx != 0) {
					m_keyPosVec[keyidx][0] -= keysize[0] / 2;
				}
				// Draw key
				src.blit(canvas, m_keyPosVec[keyidx], keysize, 1.0);
				// Update next key's position
				if (keyidx != m_numKeys - 1) {
					if (&src == &m_blkKey)
						m_keyPosVec[keyidx + 1][0] = m_keyPosVec[keyidx][0] + keysize[0] / 2;
					else
						m_keyPosVec[keyidx + 1][0] = m_keyPosVec[keyidx][0] + keysize[0];
					m_keyPosVec[keyidx + 1][1] = m_keyPosVec[keyidx][1];
				}
			}

			// Draw overlays
			char voicetxt[128]; // enough for 17 or so voices
			for (keyidx = 0; keyidx < m_numKeys; keyidx++) {
				vector<int> noteVoices = m_vm->getNoteVoices(keyidx+m_firstKey);
				if (noteVoices.size()>0) {
					CachedImage& src = getImageFromKeyIdx_(keyidx);
					keysize = getKeySize(keyidx);
					IRECT overlay{ m_keyPosVec[keyidx][0], m_keyPosVec[keyidx][1], int(m_keyPosVec[keyidx][0] + keysize[0]), int(m_keyPosVec[keyidx][1] + keysize[1]) };
					IChannelBlend blend{ IChannelBlend::kBlendNone, 0.5 };
					Color overcolor = &src == &m_blkKey ? Color(255, 255, 255, 255) : Color(0,0,0,255);
					Color textcolor = Color(255, 255, 255, 255) - overcolor;
				}
			}
			m_drawCnt++;
			mDirty = false;
			return true;
		};
	};
}
#endif

