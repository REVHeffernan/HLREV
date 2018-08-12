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

#ifndef CVGUIENTITYINFO_H
#define CVGUIENTITYINFO_H

#define ENTITY_INFO_WINDOW_X			XRES( 16 )
#define ENTITY_INFO_WINDOW_Y			YRES( 256 )
#define ENTITY_INFO_WINDOW_SIZE_X		XRES( 256 )
#define ENTITY_INFO_WINDOW_SIZE_Y		YRES( 256 )

#define ENTITY_INFO_ITEM_COLOR_R		0
#define ENTITY_INFO_ITEM_COLOR_G		0
#define ENTITY_INFO_ITEM_COLOR_B		255

#define ENTITY_INFO_ALLY_COLOR_R		0
#define ENTITY_INFO_ALLY_COLOR_G		255
#define ENTITY_INFO_ALLY_COLOR_B		0

#define ENTITY_INFO_NEUTRAL_COLOR_R		255
#define ENTITY_INFO_NEUTRAL_COLOR_G		242
#define ENTITY_INFO_NEUTRAL_COLOR_B		0

#define ENTITY_INFO_ENEMY_COLOR_R		255
#define ENTITY_INFO_ENEMY_COLOR_G		0
#define ENTITY_INFO_ENEMY_COLOR_B		0

class CVGUIEntityInfo : public CTransparentPanel
{
public:
	CVGUIEntityInfo();

	void Update();

	int MsgFunc_EntInfo( const char *pszName, int iSize, void *pbuf );

private:
	const char *GetTypeName( void );

	void SetColors( int &iRed, int &iGreen, int &iBlue );

	char *m_cEntityName;

	int m_iEntityHealth;
	int m_iEntityType;

	Label *m_pLabelName;
	Label *m_pLabelType;
	Label *m_pLabelHealth;
};

#endif // CVGUIENTITYINFO_H
