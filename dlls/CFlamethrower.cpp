/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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
#include "CFlame.h"
#include "player.h"
#include "weapons.h"

enum flamethrower_e {
	FLAMETHROWER_IDLE = 0,
	FLAMETHROWER_FIDGET,
	FLAMETHROWER_ALTFIRE1,
	FLAMETHROWER_ALTFIRE2,
	FLAMETHROWER_ALTFIRE3,
	FLAMETHROWER_FIRE1,
	FLAMETHROWER_FIRE2,
	FLAMETHROWER_FIRE3,
	FLAMETHROWER_FIRE4,
	FLAMETHROWER_DRAW,
	FLAMETHROWER_HOLSTER
};

LINK_ENTITY_TO_CLASS( weapon_flamethrower, CFlamethrower );

void CFlamethrower::Spawn( void )
{
	pev->classname = MAKE_STRING( "weapon_flamethrower" );
	Precache();
	m_iId = WEAPON_FLAMETHROWER;
	SET_MODEL( ENT( pev ), "models/w_flamethrower.mdl" );

	m_iDefaultAmmo = 100;

	FallInit();
}

void CFlamethrower::Precache( void )
{
	PRECACHE_MODEL( "models/v_flamethrower.mdl" );
	PRECACHE_MODEL( "models/w_flamethrower.mdl" );
	PRECACHE_MODEL( "models/p_flamethrower.mdl" );

	PRECACHE_SOUND( "weapons/fla_fire.wav" );

	UTIL_PrecacheOther( "flame" );

	m_usFlamethrower = PRECACHE_EVENT( 1, "events/flamethrower.sc" );
}

int CFlamethrower::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "flame";
	p->iMaxAmmo1 = FLAME_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 4;
	p->iPosition = 3;
	p->iId = m_iId = WEAPON_FLAMETHROWER;
	p->iFlags = 0;
	p->iWeight = RPG_WEIGHT;

	return 1;
}

BOOL CFlamethrower::Deploy( void )
{
	return DefaultDeploy( "models/v_flamethrower.mdl", "models/p_flamethrower.mdl", FLAMETHROWER_DRAW, "egon" );
}

void CFlamethrower::Holster( int skiplocal )
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	SendWeaponAnim( FLAMETHROWER_HOLSTER );
}

void CFlamethrower::PrimaryAttack( void )
{
	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || (m_pPlayer->pev->waterlevel == 3 && m_pPlayer->pev->watertype > CONTENT_FLYFIELD) )
	{
		if ( m_fFireOnEmpty )
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}
		return;
	}

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif
	PLAYBACK_EVENT( flags, m_pPlayer->edict(), m_usFlamethrower );

	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

#ifndef CLIENT_DLL
	Vector vecPos, vecVelocity;
	Vector vecForward, vecRight, vecUp;

	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

	vecForward = gpGlobals->v_forward;
	vecRight = gpGlobals->v_right;
	vecUp = gpGlobals->v_up;

	vecPos = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs;
	vecPos = vecPos + vecForward * 16.0;
	vecPos = vecPos + vecRight * RANDOM_FLOAT( 4.0, 6.0 );
	vecPos = vecPos + vecUp * RANDOM_FLOAT( 1.0, 2.0 );

	vecVelocity = vecForward * 400.0;
	vecVelocity = vecVelocity + vecRight * (RANDOM_LONG( 0, 1 ) ? 16.0 : -16.0);
	vecVelocity = vecVelocity + vecUp * (RANDOM_LONG( 0, 1 ) ? 16.0 : -8.0);

	CFlame::Shoot( m_pPlayer->pev, vecPos, vecVelocity );
#endif

	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.1;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10.0, 15.0 );
}

void CFlamethrower::WeaponIdle( void )
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if ( m_flTimeWeaponIdle > gpGlobals->time )
		return;

	SendWeaponAnim( RANDOM_LONG( 0, 1 ) ? FLAMETHROWER_FIDGET : FLAMETHROWER_IDLE );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0;
}

class CFlamethrowerAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache();
		SET_MODEL( ENT( pev ), "models/w_flameammo.mdl" );
		CBasePlayerAmmo::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/w_flameammo.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if ( pOther->GiveAmmo( 20, "flame", FLAME_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/9mmclip1.wav", 1.0, ATTN_NORM );
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_flameammo, CFlamethrowerAmmo );
