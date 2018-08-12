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
#include "soundent.h"
#include "gamerules.h"

enum MP5N_e
{
	MP5_LONGIDLE = 0,
	MP5_IDLE1,
	MP5_LAUNCH,
	MP5_RELOAD,
	MP5_DEPLOY,
	MP5_FIRE1,
	MP5_FIRE2,
	MP5_FIRE3,
};



LINK_ENTITY_TO_CLASS( weapon_mp5navy, CMP5Navy );


//=========================================================
//=========================================================
void CMP5Navy::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_mp5navy"); // hack to allow for old names
	Precache( );
	SET_MODEL(ENT(pev), "models/w_mp5navy.mdl");
	m_iId = WEAPON_MP5;

	m_iDefaultAmmo = MP5NAVY_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}


void CMP5Navy::Precache( void )
{
	PRECACHE_MODEL("models/v_mp5navy.mdl");
	PRECACHE_MODEL("models/w_mp5navy.mdl");
	PRECACHE_MODEL("models/p_mp5navy.mdl");

	m_iShell = PRECACHE_MODEL ("models/shell.mdl");// brass shellTE_MODEL

	PRECACHE_MODEL("models/w_9mmARclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");              

	PRECACHE_SOUND("items/clipinsert1.wav");
	PRECACHE_SOUND("items/cliprelease1.wav");
	
	PRECACHE_SOUND ("weapons/m161.wav");// H to the K
	PRECACHE_SOUND ("weapons/m162.wav");// H to the K
	PRECACHE_SOUND ("weapons/m163.wav");// H to the K

	PRECACHE_SOUND ("weapons/357_cock1.wav");

	m_usMP5N = PRECACHE_EVENT( 1, "events/mp5navy.sc" );
}

int CMP5Navy::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "MP5";
	p->iMaxAmmo1 = MP5_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = WEAPON_NOCLIP;
	p->iMaxClip = MP5NAVY_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 3;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_MP5;
	p->iWeight = MP5NAVY_WEIGHT;

	return 1;
}

BOOL CMP5Navy::Deploy( )
{
	return DefaultDeploy( "models/v_mp5navy.mdl", "models/p_mp5navy.mdl", MP5_DEPLOY, "mp5" );
}

void CMP5Navy::Holster( int skiplocal /* = 0 */)
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	if ( m_fInZoom )
		SecondaryAttack();

	SendWeaponAnim(MP5_DEPLOY); //MP5 has no holster, so just do the deploy... sigh..
}

void CMP5Navy::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3 && m_pPlayer->pev->watertype > CONTENT_FLYFIELD)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;
		return;
	}

	if (m_iClip <= 0)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;
		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_iClip--;


	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

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

	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	Vector vecDir;

	vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, Vector( 0.0, 0.0, 0.0 ), 8192, BULLET_PLAYER_MP5, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed );

  int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

//	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usMP5N, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );
	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usMP5N, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, RANDOM_LONG(MP5_FIRE1, MP5_FIRE3), m_iShell, 0, 0 );

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}



void CMP5Navy::SecondaryAttack( void )
{
	return;
}

void CMP5Navy::Reload( void )
{
	if ( m_pPlayer->ammo_mp5 <= 0 )
		return;

	m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
	m_fInZoom = 0;

	DefaultReload( MP5NAVY_MAX_CLIP, MP5_RELOAD, 2.2 );
}


void CMP5Navy::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	int iAnim;
	switch ( RANDOM_LONG( 0, 1 ) )
	{
	case 0:	
		iAnim = MP5_LONGIDLE;	
		break;
	
	default:
	case 1:
		iAnim = MP5_IDLE1;
		break;
	}

	SendWeaponAnim( iAnim );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 ); // how long till we do this again.
}
