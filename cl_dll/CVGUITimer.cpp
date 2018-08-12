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

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include "vgui_int.h"
#include "vgui_TeamFortressViewport.h"

#include "CVGUITimer.h"

CVGUITimer::CVGUITimer() : CTransparentPanel( 255, TIMER_WINDOW_X, TIMER_WINDOW_Y, TIMER_WINDOW_SIZE_X, TIMER_WINDOW_SIZE_Y )
{
	CSchemeManager *pSchemeManager = gViewPort->GetSchemeManager();
	SchemeHandle_t hSchemeText = pSchemeManager->getSchemeHandle( "Title Font" );
	m_iTime = 0;

	m_pLabel = new Label( "00:00", 0, -16, 64, 64 );
	m_pLabel->setParent( this );
	m_pLabel->setFont( pSchemeManager->getFont( hSchemeText ) );
	m_pLabel->setBgColor( 0, 0, 0, 255 );
	m_pLabel->setFgColor( 255, 255, 255, 0 );
	m_pLabel->setContentAlignment( vgui::Label::a_west );
	m_pLabel->setVisible( true );
}

void CVGUITimer::Update()
{
	if ( m_iTime == 0 )
	{
		setVisible( false );
		return;
	}

	int iMinutes = (m_iTime % 3600) / 60;
	int iSeconds = m_iTime % 60;

	setVisible( true );
	m_pLabel->setText( "%02d:%02d", iMinutes, iSeconds );
}

/*void CVGUITimer::Reset()
{
	m_iTime = 0;
	setVisible( false );
}*/

int CVGUITimer::MsgFunc_Timer( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	m_iTime = READ_LONG();

	return 1;
}
