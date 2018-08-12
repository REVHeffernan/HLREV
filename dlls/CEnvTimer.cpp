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

#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "CEnvTimer.h"

extern int gmsgTimer;

TYPEDESCRIPTION CEnvTimer::m_SaveData[] =
{
	DEFINE_FIELD( CEnvTimer, m_hActivator, FIELD_EHANDLE ),
	DEFINE_FIELD( CEnvTimer, m_iTime, FIELD_INTEGER )
};
IMPLEMENT_SAVERESTORE( CEnvTimer, CPointEntity );

LINK_ENTITY_TO_CLASS( env_timer, CEnvTimer );

void CEnvTimer::Spawn( void )
{
	pev->solid		= SOLID_NOT;
	pev->movetype	= MOVETYPE_NONE;

	SetThink( &CEnvTimer::TimeThink );
	pev->nextthink = gpGlobals->time + 1.0f;
}

void CEnvTimer::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !pActivator )
		return;

	if ( !pActivator->IsPlayer() )
		return;

	m_iTime			= pev->health;
	m_hActivator	= pActivator;
}

void CEnvTimer::TimeThink( void )
{
	if ( !m_hActivator )
	{
		pev->nextthink = gpGlobals->time + 1.0f;
		return;
	}

	MESSAGE_BEGIN( MSG_ONE, gmsgTimer, NULL, m_hActivator->pev );
		WRITE_LONG( m_iTime );
	MESSAGE_END();
	m_iTime--;

	if ( m_iTime < 0 )
		m_hActivator = nullptr;

	pev->nextthink = gpGlobals->time + 1.0f;
}
