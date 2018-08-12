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

enum herb_red_e {
	HERB_RED_IDLE1 = 0,
	HERB_RED_IDLE2,
	HERB_RED_USE,
	HERB_RED_DRAW
};

LINK_ENTITY_TO_CLASS( weapon_herb_red, CHerbRed );

void CHerbRed::Spawn( void )
{
	pev->classname = MAKE_STRING( "weapon_herb_red" );
	Precache();
	m_iId = WEAPON_HERB_RED;
	SET_MODEL( ENT( pev ), "models/w_herb_red.mdl" );

	m_iDefaultAmmo = RPG_DEFAULT_GIVE;

	FallInit();
}

void CHerbRed::Precache( void )
{
	PRECACHE_MODEL( "models/v_herb_red.mdl" );
	PRECACHE_MODEL( "models/w_herb_red.mdl" );
	PRECACHE_MODEL( "models/p_herb_red.mdl" );
}

int CHerbRed::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "HerbRed";
	p->iMaxAmmo1 = HEALTH_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 3;
	p->iPosition = 1;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;
	p->iId = m_iId = WEAPON_HERB_RED;
	p->iWeight = RPG_WEIGHT;

	return 1;
}

BOOL CHerbRed::Deploy( void )
{
	m_flReleaseThrow = -1.0;
	return DefaultDeploy( "models/v_herb_red.mdl", "models/p_herb_red.mdl", HERB_RED_DRAW, "crowbar", 0 );
}

BOOL CHerbRed::CanHolster( void )
{
	return m_flStartThrow == 0.0;
}

void CHerbRed::Holster( int skiplocal )
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	if ( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
	{
		SendWeaponAnim( HERB_RED_USE );
	}
	else
	{
		m_pPlayer->pev->weapons &= ~(1 << WEAPON_HERB_RED);
		SetThink( &CHerbRed::DestroyItem );
		SetNextThink( 0.1 );
	}
}

void CHerbRed::PrimaryAttack( void )
{
	if ( m_pPlayer->pev->health >= 100 )
		return;

	if ( !m_flStartThrow && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] > 0 )
	{
		m_flStartThrow = gpGlobals->time;
		m_flReleaseThrow = 0.0;

		SendWeaponAnim( HERB_RED_USE );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
	}
}

void CHerbRed::WeaponIdle( void )
{
	if ( m_flReleaseThrow == 0.0 && m_flStartThrow )
		m_flReleaseThrow = gpGlobals->time;

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	if ( m_flStartThrow )
	{
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

		m_flReleaseThrow = 0.0;
		m_flStartThrow = 0.0;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
		m_pPlayer->TakeHealth( 50.0, DMG_GENERIC );

		if ( !m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
			m_flTimeWeaponIdle = m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;

		return;
	}
	else if ( m_flReleaseThrow > 0.0 )
	{
		m_flStartThrow = 0.0;

		if ( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
		{
			SendWeaponAnim( HERB_RED_DRAW );
		}
		else
		{
			RetireWeapon();
			return;
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
		m_flReleaseThrow = -1.0;
		return;
	}

	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
	{
		int iAnim = RANDOM_LONG( 0, 1 );
		if ( iAnim == 1 )
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
		else 
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0;

		SendWeaponAnim( iAnim );
	}
}
