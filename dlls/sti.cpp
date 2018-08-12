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
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

enum STI_e
{
	STI_IDLE = 0,
	STI_IDLE2,
	STI_IDLE3,
	STI_ATTACK,
	STI_ATTACK_SLIDEBACK,
	STI_RELOAD_SLIDEBACK,
	STI_RELOAD,
	STI_DRAW,
	STI_HOLSTER
};

LINK_ENTITY_TO_CLASS( weapon_sti, CSTI );

void CSTI::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_sti"); // hack to allow for old names
	Precache( );
	m_iId = WEAPON_STI;
	SET_MODEL(ENT(pev), "models/w_sti.mdl");

	m_iDefaultAmmo = STI_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}

void CSTI::Holster( int skiplocal )
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	if ( m_fInZoom )
		SecondaryAttack();

	SendWeaponAnim( STI_HOLSTER );
}

void CSTI::Precache( void )
{
	PRECACHE_MODEL("models/v_sti.mdl");
	PRECACHE_MODEL("models/w_sti.mdl");
	PRECACHE_MODEL("models/p_sti.mdl");

	m_iShell = PRECACHE_MODEL ("models/shell.mdl");// brass shell

	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("items/9mmclip2.wav");

	PRECACHE_SOUND ("weapons/sti_fire.wav");//silenced handgun

	m_usFireSTI = PRECACHE_EVENT( 1, "events/sti.sc" );
}

int CSTI::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = STI_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 2;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_STI;
	p->iWeight = STI_WEIGHT;

	return 1;
}

BOOL CSTI::Deploy( )
{
	// pev->body = 1;
	return DefaultDeploy( "models/v_sti.mdl", "models/p_sti.mdl", STI_DRAW, "onehanded", /*UseDecrement() ? 1 : 0*/ 0 );
}

void CSTI::PrimaryAttack( void )
{
	GlockFire( 0.01, 0.3, TRUE );
}

void CSTI::GlockFire( float flSpread , float flCycleTime, BOOL fUseAutoAim )
{
	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
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

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	#ifndef CLIENT_DLL 
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
       WRITE_BYTE( TE_DLIGHT );
       WRITE_COORD( pev->origin.x ); // origin
       WRITE_COORD( pev->origin.y );
       WRITE_COORD( pev->origin.z );
       WRITE_BYTE( 16 );     // radius
       WRITE_BYTE( 255 );    // R
       WRITE_BYTE( 255 );    // G
       WRITE_BYTE( 160 );    // B
       WRITE_BYTE( 0 );      // life * 10
       WRITE_BYTE( 0 );      // decay
    MESSAGE_END();
	#endif 

	// silenced
	if (pev->body == 1)
	{
		m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;
	}
	else
	{
		// non-silenced
		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
	}

	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming;
	
	if ( fUseAutoAim )
	{
		vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
	}
	else
	{
		vecAiming = gpGlobals->v_forward;
	}

	Vector vecDir;
	vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, m_fInZoom ? Vector(0,0,0) : Vector( flSpread, flSpread, flSpread ), 8192, BULLET_PLAYER_9MM, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFireSTI, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}


void CSTI::Reload( void )
{
	if ( m_pPlayer->ammo_9mm <= 0 )
		 return;

	int iResult;

	if (m_iClip == 0)
		iResult = DefaultReload( 12, STI_RELOAD_SLIDEBACK, 2.8 );
	else
		iResult = DefaultReload( 12, STI_RELOAD, 2.8 );

	if (iResult)
	{
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
		m_fInZoom = 0;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
	}
}

void CSTI::SecondaryAttack( void )
{
	if ( m_pPlayer->pev->fov != 0 )
	{
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
		m_fInZoom = 0;
	}
	else if ( m_pPlayer->pev->fov != 20 )
	{
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 20;
		m_fInZoom = 1;
	}

	SetNextThink( 0.1 );
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0;
}

void CSTI::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	// only idle if the slid isn't back
	if (m_iClip != 0)
	{
		int iAnim;
		float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0.0, 1.0 );

		if (flRand <= 0.3 + 0 * 0.75)
		{
			iAnim = STI_IDLE3;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 49.0 / 16;
		}
		else if (flRand <= 0.6 + 0 * 0.875)
		{
			iAnim = STI_IDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0 / 16.0;
		}
		else
		{
			iAnim = STI_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 40.0 / 16.0;
		}
		SendWeaponAnim( iAnim, 1 );
	}
}
