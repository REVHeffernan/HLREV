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

#include "CVGUIEntityInfo.h"

CVGUIEntityInfo::CVGUIEntityInfo() : CTransparentPanel( 255, ENTITY_INFO_WINDOW_X, ENTITY_INFO_WINDOW_Y, ENTITY_INFO_WINDOW_SIZE_X, ENTITY_INFO_WINDOW_SIZE_Y )
{
	CSchemeManager *pSchemeManager = gViewPort->GetSchemeManager();
	SchemeHandle_t hSchemeText = pSchemeManager->getSchemeHandle( "Briefing Text" );

	int iRed, iGreen, iBlue;
	SetColors( iRed, iGreen, iBlue );
	m_cEntityName = "";
	m_iEntityType = 0;
	m_iEntityHealth = 0;

	m_pLabelName = new Label( gHUD.m_TextMessage.BufferedLocaliseTextString( "#EntInfo_Name_Label" ), XRES( 0 ), YRES( 0 ) );
	m_pLabelName->setParent( this );
	m_pLabelName->setFont( pSchemeManager->getFont( hSchemeText ) );
	m_pLabelName->setBgColor( 0, 0, 0, 255 );
	m_pLabelName->setFgColor( iRed, iGreen, iBlue, 255 );
	m_pLabelName->setContentAlignment( vgui::Label::a_west );

	m_pLabelType = new Label( gHUD.m_TextMessage.BufferedLocaliseTextString( "#EntInfo_Type_Label" ), XRES( 0 ), YRES( 16 ) );
	m_pLabelType->setParent( this );
	m_pLabelType->setFont( pSchemeManager->getFont( hSchemeText ) );
	m_pLabelType->setBgColor( 0, 0, 0, 255 );
	m_pLabelType->setFgColor( iRed, iGreen, iBlue, 255 );
	m_pLabelType->setContentAlignment( vgui::Label::a_west );

	m_pLabelHealth = new Label( gHUD.m_TextMessage.BufferedLocaliseTextString( "#EntInfo_Health_Label" ), XRES( 0 ), YRES( 32 ) );
	m_pLabelHealth->setParent( this );
	m_pLabelHealth->setFont( pSchemeManager->getFont( hSchemeText ) );
	m_pLabelHealth->setBgColor( 0, 0, 0, 255 );
	m_pLabelHealth->setFgColor( iRed, iGreen, iBlue, 255 );
	m_pLabelHealth->setContentAlignment( vgui::Label::a_west );
}

void CVGUIEntityInfo::Update()
{
	int iRed, iGreen, iBlue;
	SetColors( iRed, iGreen, iBlue );

	if ( !strcmp( "", m_cEntityName ) )
	{
		m_iEntityType = 0;
		m_iEntityHealth = 0;
		m_pLabelName->setFgColor( iRed, iGreen, iBlue, 255 );
		m_pLabelType->setFgColor( iRed, iGreen, iBlue, 255 );
		m_pLabelHealth->setFgColor( iRed, iGreen, iBlue, 255 );
		return;
	}

	char cName[256] = "";
	strcpy( cName, gHUD.m_TextMessage.BufferedLocaliseTextString( "#EntInfo_Name_Label" ) );
	strcat( cName, " " );
	if ( strstr( m_cEntityName, "#" ) != NULL )
		strcat( cName, gHUD.m_TextMessage.BufferedLocaliseTextString( m_cEntityName ) );
	else
		strcat( cName, m_cEntityName );

	m_pLabelName->setText( cName );
	m_pLabelName->setFgColor( iRed, iGreen, iBlue, 0 );

	char cType[64] = "";
	strcpy( cType, gHUD.m_TextMessage.BufferedLocaliseTextString( "#EntInfo_Type_Label" ) );
	strcat( cType, " " );
	strcat( cType, GetTypeName() );
	m_pLabelType->setText( cType );
	m_pLabelType->setFgColor( iRed, iGreen, iBlue, 0 );

	if ( m_iEntityType != 0 )
	{
		char cHealth[64] = "";
		if ( m_iEntityHealth > 0 )
		{
			sprintf( cHealth, "%s %d", gHUD.m_TextMessage.BufferedLocaliseTextString( "#EntInfo_Health_Label" ), m_iEntityHealth );
		}
		else
		{
			strcpy( cHealth, gHUD.m_TextMessage.BufferedLocaliseTextString( "#EntInfo_Health_Label" ) );
			strcat( cHealth, " " );
			strcat( cHealth, gHUD.m_TextMessage.BufferedLocaliseTextString( "#EntInfo_Health_Dead" ) );
		}
		m_pLabelHealth->setText( cHealth );
		m_pLabelHealth->setFgColor( iRed, iGreen, iBlue, 0 );
	}
	else
	{
		m_pLabelHealth->setText( gHUD.m_TextMessage.BufferedLocaliseTextString( "#EntInfo_Health_Label" ) );
		m_pLabelHealth->setFgColor( iRed, iGreen, iBlue, 255 );
	}
}

int CVGUIEntityInfo::MsgFunc_EntInfo( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	m_cEntityName = READ_STRING();
	m_iEntityHealth = READ_LONG();
	m_iEntityType = READ_BYTE();

	return 1;
}

const char *CVGUIEntityInfo::GetTypeName( void )
{
	switch ( m_iEntityType )
	{
	case 0: return gHUD.m_TextMessage.BufferedLocaliseTextString( "#EntInfo_Item_Type" ); break;
	case 1: return gHUD.m_TextMessage.BufferedLocaliseTextString( "#EntInfo_Ally_Type" ); break;
	case 3: return gHUD.m_TextMessage.BufferedLocaliseTextString( "#EntInfo_Enemy_Type" ); break;
	default: return gHUD.m_TextMessage.BufferedLocaliseTextString( "#EntInfo_Neutral_Type" );
	}
}

void CVGUIEntityInfo::SetColors( int &iRed, int &iGreen, int &iBlue )
{
	switch ( m_iEntityType )
	{
	case 0: // Item
		iRed = ENTITY_INFO_ITEM_COLOR_R;
		iGreen = ENTITY_INFO_ITEM_COLOR_G;
		iBlue = ENTITY_INFO_ITEM_COLOR_B;
		break;
	case 1: // Ally
		iRed = ENTITY_INFO_ALLY_COLOR_R;
		iGreen = ENTITY_INFO_ALLY_COLOR_G;
		iBlue = ENTITY_INFO_ALLY_COLOR_B;
		break;
	case 2: // Neutral
		iRed = ENTITY_INFO_NEUTRAL_COLOR_R;
		iGreen = ENTITY_INFO_NEUTRAL_COLOR_G;
		iBlue = ENTITY_INFO_NEUTRAL_COLOR_B;
		break;
	case 3: // Enemy
		iRed = ENTITY_INFO_ENEMY_COLOR_R;
		iGreen = ENTITY_INFO_ENEMY_COLOR_G;
		iBlue = ENTITY_INFO_ENEMY_COLOR_B;
		break;
	}
}
