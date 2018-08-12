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

enum glauncher_e {
	GLAUNCHER_IDLE = 0,
	GLAUNCHER_RELOAD,
	GLAUNCHER_FIRE,
	GLAUNCHER_DRAW
};

LINK_ENTITY_TO_CLASS( weapon_grenadelauncher, CGrenadeLauncher );

void CGrenadeLauncher::Spawn( void )
{
	pev->classname = MAKE_STRING( "weapon_grenadelauncher" );
	Precache();
	m_iId = WEAPON_GRENADELAUNCHER;
	SET_MODEL( ENT( pev ), "models/w_glauncher.mdl" );

	m_iDefaultAmmo = RPG_DEFAULT_GIVE;

	FallInit();
}

void CGrenadeLauncher::Precache( void )
{
	PRECACHE_MODEL( "models/v_glauncher.mdl" );
	PRECACHE_MODEL( "models/w_glauncher.mdl" );
	PRECACHE_MODEL( "models/p_glauncher.mdl" );

	PRECACHE_SOUND( "weapons/glauncher.wav" );
	PRECACHE_SOUND( "weapons/glauncher2.wav" );

	m_usGrenadeLauncher = PRECACHE_EVENT( 1, "events/glauncher.sc" );
}

int CGrenadeLauncher::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ARgrenades";
	p->iMaxAmmo1 = M203_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = RPG_MAX_CLIP;
	p->iSlot = 4;
	p->iPosition = 1;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_GRENADELAUNCHER;
	p->iWeight = RPG_WEIGHT;

	return 1;
}

BOOL CGrenadeLauncher::Deploy( void )
{
	return DefaultDeploy( "models/v_glauncher.mdl", "models/p_glauncher.mdl", GLAUNCHER_DRAW, "mp5", 0 );
}

void CGrenadeLauncher::Holster( int skiplocal )
{
	m_fInReload = FALSE;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
}

void CGrenadeLauncher::PrimaryAttack( void )
{
	if ( m_iClip <= 0 || (m_pPlayer->pev->waterlevel == 3 && m_pPlayer->pev->watertype > CONTENT_FLYFIELD) )
	{
		if ( m_fFireOnEmpty )
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}
		return;
	}

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif
	PLAYBACK_EVENT( flags, m_pPlayer->edict(), m_usGrenadeLauncher );

	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );
	CGrenade::ShootContact( m_pPlayer->pev, m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16, gpGlobals->v_forward * 800 );
	m_pPlayer->pev->punchangle.x -= 10;

	if ( !m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );

	SetNextThink( 0.1 );
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10.0, 15.0 );
}


void CGrenadeLauncher::Reload( void )
{
	if ( m_pPlayer->ammo_argrens <= 0 )
		 return;

	if ( DefaultReload( RPG_MAX_CLIP, GLAUNCHER_RELOAD, 2.5 ) )
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CGrenadeLauncher::WeaponIdle( void )
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0;
}

class CGrenadeLauncherAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache();
		SET_MODEL( ENT( pev ), "models/w_ARgrenade.mdl" );
		CBasePlayerAmmo::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/w_ARgrenade.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if ( pOther->GiveAmmo( AMMO_RPGCLIP_GIVE, "ARgrenades", M203_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM );
			return TRUE;
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS( ammo_ARgrenades, CGrenadeLauncherAmmo );
LINK_ENTITY_TO_CLASS( ammo_mp5grenades, CGrenadeLauncherAmmo );
