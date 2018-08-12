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

enum Minigun_e
{
	TEC9_IDLE1 = 0,
	TEC9_IDLE2,
	TEC9_RELOAD_SLIDEBACK,
	TEC9_RELOAD,
	TEC9_DEPLOY,
	TEC9_FIRE1,
	TEC9_FIRE2,
	TEC9_FIRE3,
	TEC9_FIRE_SLIDEBACK
};



LINK_ENTITY_TO_CLASS( weapon_minigun, CMinigun );



//=========================================================
//=========================================================
void CMinigun::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_minigun"); // hack to allow for old names
	Precache( );
	SET_MODEL(ENT(pev), "models/w_minigun.mdl");
	m_iId = WEAPON_MINIGUN;

	m_iDefaultAmmo = 999;

	FallInit();// get ready to fall down.
}


void CMinigun::Precache( void )
{
	PRECACHE_MODEL("models/v_minigun.mdl");
	PRECACHE_MODEL("models/w_minigun.mdl");
	PRECACHE_MODEL("models/p_minigun.mdl");

	m_iShell = PRECACHE_MODEL ("models/shell.mdl");// brass shellTE_MODEL

	PRECACHE_MODEL("models/w_9mmARclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");              

	PRECACHE_SOUND("items/clipinsert1.wav");
	PRECACHE_SOUND("items/cliprelease1.wav");

	PRECACHE_SOUND ("weapons/hks1.wav");// H to the K
	PRECACHE_SOUND ("weapons/hks2.wav");// H to the K
	PRECACHE_SOUND ("weapons/hks3.wav");// H to the K

	PRECACHE_SOUND ("weapons/357_cock1.wav");

	m_Minigun = PRECACHE_EVENT( 1, "events/minigun.sc" );
}

int CMinigun::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Minigun";
	p->iMaxAmmo1 = 99;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = WEAPON_NOCLIP;
	p->iMaxClip = 99;
	p->iSlot = 4;
	p->iPosition = 5;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_MINIGUN;
	p->iWeight = MP5_WEIGHT;

	return 1;
}

BOOL CMinigun::Deploy( )
{
	return DefaultDeploy( "models/v_Minigun.mdl", "models/p_Minigun.mdl", TEC9_DEPLOY, "Minigun" );
}

void CMinigun::Holster( int skiplocal /* = 0 */)
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	SendWeaponAnim(TEC9_DEPLOY); //no holster anim for mac11!
}

void CMinigun::PrimaryAttack()
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

	//m_iClip--;


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

	// single player spread
	vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_3DEGREES, 8192, BULLET_PLAYER_MP5, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );

  int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

//	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usMP5, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );
	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_Minigun, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, RANDOM_LONG(TEC9_FIRE1, TEC9_FIRE3), m_iShell, 0, 0);

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.06;

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.06;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CMinigun::Reload( void )
{
	if ( m_pPlayer->ammo_mp5 <= 0 )
		return;

	int iAnim;

	if (m_iClip == 0)
		iAnim = TEC9_RELOAD_SLIDEBACK;
	else
		iAnim = TEC9_RELOAD;

	DefaultReload( MP5_MAX_CLIP, iAnim, 1.5 );
}


void CMinigun::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	// only idle if the slid isn't back
	if (m_iClip != 0)
	{
		SendWeaponAnim(TEC9_IDLE1);

		m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15); // how long till we do this again.
	}
}



class CMinigunAmmoClip : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_9mmARclip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_9mmARclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( AMMO_MP5CLIP_GIVE, "Minigun", MP5_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 0, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_Minigun, CMinigunAmmoClip );




class CMinigunChainammo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_chainammo.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_chainammo.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( AMMO_CHAINBOX_GIVE, "9mm", _9MM_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 0, ATTN_NORM);
		}
		return bResult;
	}
};

