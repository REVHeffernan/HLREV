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
#include "player.h"
#include "weapons.h"

enum coltsaa_e {
	COLTSAA_IDLE1 = 0,
	COLTSAA_FIDGET,
	COLTSAA_FIRE,
	COLTSAA_FIRE_LAST,
	COLTSAA_RELOAD,
	COLTSAA_RELOAD_EMPTY,
	COLTSAA_HOLSTER,
	COLTSAA_DRAW,
	COLTSAA_IDLE2,
	COLTSAA_IDLE3
};

LINK_ENTITY_TO_CLASS( weapon_python, CColtSAA );
LINK_ENTITY_TO_CLASS( weapon_357, CColtSAA );

int CColtSAA::GetItemInfo( ItemInfo *p )
{
 	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "357";
	p->iMaxAmmo1 = _357_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 6;
	p->iFlags = 0;
	p->iSlot = 1;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_COLT_SAA;
	p->iWeight = COLT_SAA_WEIGHT;
	return 1;
}

void CColtSAA::Spawn( void )
{
	pev->classname = MAKE_STRING( "weapon_357" );
	Precache();
	m_iId = WEAPON_COLT_SAA;
	SET_MODEL( ENT( pev ), "models/w_357.mdl" );
	m_iDefaultAmmo = 6;
	FallInit();
}

void CColtSAA::Precache( void )
{
	PRECACHE_MODEL( "models/v_357.mdl" );
	PRECACHE_MODEL( "models/w_357.mdl" );
	PRECACHE_MODEL( "models/p_357.mdl" );         

	m_iShell = PRECACHE_MODEL ("models/shell.mdl");// brass shellTE_MODEL

	PRECACHE_SOUND( "weapons/357_shot1.wav" );
	PRECACHE_SOUND( "weapons/357_shot2.wav" );

	m_usFireColtSAA = PRECACHE_EVENT( 1, "events/python.sc" );
}

BOOL CColtSAA::Deploy( void )
{
	return DefaultDeploy( "models/v_357.mdl", "models/p_357.mdl", COLTSAA_DRAW, "python", UseDecrement(), 0 );
}

void CColtSAA::Holster( int skiplocal )
{
	m_fInReload = FALSE;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
	SendWeaponAnim( COLTSAA_HOLSTER );
}

void CColtSAA::PrimaryAttack( void )
{
	// don't fire underwater
	if ( m_pPlayer->pev->waterlevel == 3 && m_pPlayer->pev->watertype > CONTENT_FLYFIELD )
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	if (m_iClip <= 0)
	{
		if (!m_fFireOnEmpty)
			Reload( );
		else
		{
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/357_cock1.wav", 0.8, ATTN_NORM);
			m_flNextPrimaryAttack = 0.15;
		}

		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

#ifndef CLIENT_DLL 
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
       WRITE_BYTE( TE_DLIGHT );
       WRITE_COORD( pev->origin.x ); // origin
       WRITE_COORD( pev->origin.y );
       WRITE_COORD( pev->origin.z );
       WRITE_BYTE( 16 );     // radius
       WRITE_BYTE( 255 );    // R
       WRITE_BYTE( 255 );    // G
       WRITE_BYTE( 180 );    // B
       WRITE_BYTE( 0 );      // life * 10
       WRITE_BYTE( 0 );      // decay
    MESSAGE_END();
#endif 

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );


	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

	Vector vecSrc = m_pPlayer->GetGunPosition( );
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
	Vector vecDir;
	vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_357, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed);

    int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif
	//PLAYBACK_EVENT( flags, m_pPlayer->edict(), m_usFireColtSAA );
	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireColtSAA, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, m_iClip ? COLTSAA_FIRE_LAST : COLTSAA_FIRE, m_iShell, 0, 0);
	
	if ( !m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.75;
	m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CColtSAA::Reload( void )
{
	if ( m_pPlayer->ammo_357 <= 0 )
		return;

	int iAnim;

	if (m_iClip == 0)
		iAnim = COLTSAA_RELOAD_EMPTY;
	else
		iAnim = COLTSAA_RELOAD;

	if ( DefaultReload( 6, iAnim, 2.0, 0 ) )
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10.0, 15.0 );
}

void CColtSAA::WeaponIdle( void )
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	// only idle if the slid isn't back
	if (m_iClip != 0)
	{
		int iAnim;
		float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10.0, 15.0);
		if (flRand <= 0.5)
		{
			iAnim = COLTSAA_IDLE1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 70.0 / 30.0;
		}
		else if (flRand <= 0.7)
		{
			iAnim = COLTSAA_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0 / 30.0;
		}
		else if (flRand <= 0.9)
		{
			iAnim = COLTSAA_IDLE3;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 88.0 / 30.0;
		}
		else
		{
			iAnim = COLTSAA_FIDGET;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 170.0 / 30.0;
		}

		SendWeaponAnim(iAnim, 0);
	}
}

class CColtSAAAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache();
		SET_MODEL( ENT( pev ), "models/w_357ammobox.mdl" );
		CBasePlayerAmmo::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/w_357ammobox.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if ( pOther->GiveAmmo( 6, "357", _357_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/9mmclip1.wav", 1.0, ATTN_NORM );
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_357, CColtSAAAmmo );
