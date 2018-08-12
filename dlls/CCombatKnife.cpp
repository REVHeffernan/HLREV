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
#include "gamerules.h"
#include "player.h"
#include "skill.h"
#include "weapons.h"

#define	COMBAT_KNIFE_BODYHIT_VOLUME 128
#define	COMBAT_KNIFE_WALLHIT_VOLUME 512

enum combatknife_e {
	COMBATKNIFE_IDLE = 0,
	COMBATKNIFE_DRAW,
	COMBATKNIFE_HOLSTER,
	COMBATKNIFE_ATTACK1HIT,
	COMBATKNIFE_ATTACK1MISS,
	COMBATKNIFE_ATTACK2MISS,
	COMBATKNIFE_ATTACK2HIT,
	COMBATKNIFE_ATTACK3MISS,
	COMBATKNIFE_ATTACK3HIT,
	COMBATKNIFE_IDLE2,
	COMBATKNIFE_IDLE3
};

LINK_ENTITY_TO_CLASS( weapon_crowbar, CCombatKnife );

void CCombatKnife::Spawn( void )
{
	pev->classname = MAKE_STRING( "weapon_crowbar" );
	Precache();
	m_iId = WEAPON_COMBAT_KNIFE;
	SET_MODEL( ENT( pev ), "models/w_crowbar.mdl" );
	m_iClip = -1;
	FallInit();
}

void CCombatKnife::Precache( void )
{
	PRECACHE_MODEL( "models/v_crowbar.mdl" );
	PRECACHE_MODEL( "models/w_crowbar.mdl" );
	PRECACHE_MODEL( "models/p_crowbar.mdl" );

	PRECACHE_SOUND( "weapons/cbar_hit1.wav" );
	PRECACHE_SOUND( "weapons/cbar_hit2.wav" );
	PRECACHE_SOUND( "weapons/cbar_hitbod1.wav" );
	PRECACHE_SOUND( "weapons/cbar_hitbod2.wav" );
	PRECACHE_SOUND( "weapons/cbar_hitbod3.wav" );
	PRECACHE_SOUND( "weapons/cbar_miss1.wav" );
}

int CCombatKnife::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 0;
	p->iId = WEAPON_COMBAT_KNIFE;
	p->iWeight = 0;
	return 1;
}

BOOL CCombatKnife::Deploy( void )
{
	return DefaultDeploy( "models/v_crowbar.mdl", "models/p_crowbar.mdl", COMBATKNIFE_DRAW, "crowbar" );
}

void CCombatKnife::Holster( int skiplocal )
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	SendWeaponAnim( COMBATKNIFE_HOLSTER );
}

void FindHullIntersection( const Vector &vecSrc, TraceResult &tr, float *mins, float *maxs, edict_t *pEntity )
{
	int			i, j, k;
	float		distance;
	float		*minmaxs[2] = {mins, maxs};
	TraceResult tmpTrace;
	Vector		vecHullEnd = tr.vecEndPos;
	Vector		vecEnd;

	distance = 1e6f;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc)*2);
	UTIL_TraceLine( vecSrc, vecHullEnd, dont_ignore_monsters, pEntity, &tmpTrace );
	if ( tmpTrace.flFraction < 1.0 )
	{
		tr = tmpTrace;
		return;
	}

	for ( i = 0; i < 2; i++ )
	{
		for ( j = 0; j < 2; j++ )
		{
			for ( k = 0; k < 2; k++ )
			{
				vecEnd.x = vecHullEnd.x + minmaxs[i][0];
				vecEnd.y = vecHullEnd.y + minmaxs[j][1];
				vecEnd.z = vecHullEnd.z + minmaxs[k][2];

				UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, pEntity, &tmpTrace );
				if ( tmpTrace.flFraction < 1.0 )
				{
					float thisDistance = (tmpTrace.vecEndPos - vecSrc).Length();
					if ( thisDistance < distance )
					{
						tr = tmpTrace;
						distance = thisDistance;
					}
				}
			}
		}
	}
}

void CCombatKnife::PrimaryAttack( void )
{
	if ( !Swing( 1 ) )
	{
		SetThink( &CCombatKnife::SwingAgain );
		SetNextThink( 0.1 );
	}
}

int CCombatKnife::Swing( int fFirst )
{
	int fDidHit = FALSE;

	TraceResult tr;

	UTIL_MakeVectors( m_pPlayer->pev->v_angle );
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 32.0;

	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, ENT( m_pPlayer->pev ), &tr );
#ifndef CLIENT_DLL
	if ( tr.flFraction >= 1.0 )
	{
		UTIL_TraceHull( vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT( m_pPlayer->pev ), &tr );
		if ( tr.flFraction < 1.0 )
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );
			if ( !pHit || pHit->IsBSPModel() )
				FindHullIntersection( vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict() );

			vecEnd = tr.vecEndPos;	// This is the point on the actual surface (the hull could have hit space)
		}
	}
#endif

	if ( tr.flFraction >= 1.0 )
	{
		if ( fFirst )
		{
			// Miss
			switch( (m_iSwing++) % 3 )
			{
			case 0: SendWeaponAnim( COMBATKNIFE_ATTACK1MISS ); break;
			case 1: SendWeaponAnim( COMBATKNIFE_ATTACK2MISS ); break;
			case 2: SendWeaponAnim( COMBATKNIFE_ATTACK3MISS ); break;
			}
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0;

			// play wiff or swish sound
			EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/cbar_miss1.wav", 1.0, ATTN_NORM, 0, 94 + RANDOM_LONG( 0, 0xF ) );

			// player "shoot" animation
			m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
		}
	}
	else
	{
		switch( ((m_iSwing++) % 2) + 1 )
		{
		case 0: SendWeaponAnim( COMBATKNIFE_ATTACK1HIT ); break;
		case 1: SendWeaponAnim( COMBATKNIFE_ATTACK2HIT ); break;
		case 2: SendWeaponAnim( COMBATKNIFE_ATTACK3HIT ); break;
		}

		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
		
#ifndef CLIENT_DLL
		// hit
		fDidHit = TRUE;
		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );

		ClearMultiDamage();

		if ( (m_flNextPrimaryAttack + 1 < UTIL_WeaponTimeBase()) || g_pGameRules->IsMultiplayer() )
		{
			// first swing does full damage
			pEntity->TraceAttack( m_pPlayer->pev, gSkillData.plrDmgCombatKnife, gpGlobals->v_forward, &tr, DMG_CLUB ); 
		}
		else
		{
			// subsequent swings do half
			pEntity->TraceAttack( m_pPlayer->pev, gSkillData.plrDmgCombatKnife / 2, gpGlobals->v_forward, &tr, DMG_CLUB ); 
		}	
		ApplyMultiDamage( m_pPlayer->pev, m_pPlayer->pev );

		// play thwack, smack, or dong sound
		float flVol = 1.0;
		int fHitWorld = TRUE;

		if ( pEntity )
		{
			if ( pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE )
			{
				// play thwack or smack sound
				switch ( RANDOM_LONG( 0, 2 ) )
				{
				case 0: EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/cbar_hitbod1.wav", 1.0, ATTN_NORM ); break;
				case 1: EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/cbar_hitbod2.wav", 1.0, ATTN_NORM ); break;
				case 2: EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/cbar_hitbod3.wav", 1.0, ATTN_NORM ); break;
				}

				m_pPlayer->m_iWeaponVolume = COMBAT_KNIFE_BODYHIT_VOLUME;

				if ( !pEntity->IsAlive() )
				{
					m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5; //LRC: corrected half-life bug
					return TRUE;
				}
				else
					flVol = 0.1;

				fHitWorld = FALSE;
			}
		}

		// play texture hit sound
		// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line

		if ( fHitWorld )
		{
			float fvolbar = TEXTURETYPE_PlaySound( &tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2.0, BULLET_PLAYER_COMBAT_KNIFE );

			if ( g_pGameRules->IsMultiplayer() )
			{
				// override the volume here, cause we don't play texture sounds in multiplayer, 
				// and fvolbar is going to be 0 from the above call.

				fvolbar = 1;
			}

			// also play crowbar strike
			switch( RANDOM_LONG( 0, 1 ) )
			{
			case 0: EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/cbar_hit1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG( 0, 3 ) ); break;
			case 1: EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/cbar_hit2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG( 0, 3 ) ); break;
			}

			// delay the decal a bit
			m_pTRHit = tr;
		}

		m_pPlayer->m_iWeaponVolume = flVol * COMBAT_KNIFE_WALLHIT_VOLUME;
#endif
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;
		
		SetThink( &CCombatKnife::Smack );
		SetNextThink( 0.2 );
	}
	return fDidHit;
}

void CCombatKnife::SwingAgain( void )
{
	Swing( 0 );
}

void CCombatKnife::Smack( void )
{
	DecalGunshot( &m_pTRHit, BULLET_PLAYER_COMBAT_KNIFE );
}

void CCombatKnife::WeaponIdle( void )
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
		if (flRand <= 0.33)
		{
			iAnim = COMBATKNIFE_IDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 36.0 / 13.0;
		}
		else if (flRand <= 0.66)
		{
			iAnim = COMBATKNIFE_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 81.0 / 15.0;
		}
		else
		{
			iAnim = COMBATKNIFE_IDLE3;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 81.0 / 15.0;
		}

		SendWeaponAnim(iAnim, 0);
	}
}
