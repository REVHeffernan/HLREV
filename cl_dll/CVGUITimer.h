/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#ifndef CVGUITIMER_H
#define CVGUITIMER_H

#define TIMER_WINDOW_SIZE_X		XRES( 48 )
#define TIMER_WINDOW_SIZE_Y		YRES( 48 )

#define TIMER_WINDOW_X			((ScreenWidth / 2) - (TIMER_WINDOW_SIZE_X / 2))
#define TIMER_WINDOW_Y			0

class CVGUITimer : public CTransparentPanel
{
public:
	CVGUITimer();
	void Update( void );

	int MsgFunc_Timer( const char *pszName, int iSize, void *pbuf );

private:
	int m_iTime;

	Label *m_pLabel;
};

#endif // CVGUITIMER_H
