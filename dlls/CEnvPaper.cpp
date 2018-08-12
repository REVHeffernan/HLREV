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

#include "CEnvPaper.h"

#define MAX_MESSAGE_CHUNK	60
#define MAX_MESSAGE_LENGTH	1536

LINK_ENTITY_TO_CLASS( env_paper, CEnvPaper );

extern int gmsgPaperTitle;
extern int gmsgPaper;

void CEnvPaper::Spawn( void )
{
	pev->solid		= SOLID_NOT;
	pev->movetype	= MOVETYPE_NONE;
}

void CEnvPaper::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBaseEntity *pPlayer = NULL;

	// Determine the activator
	if ( pActivator && pActivator->IsPlayer() )
		pPlayer = pActivator;
	else
		pPlayer = CBaseEntity::Instance( g_engfuncs.pfnPEntityOfEntIndex( 1 ) );

	if ( pPlayer )
	{
		// Send the paper's title
		MESSAGE_BEGIN( MSG_ONE, gmsgPaperTitle, NULL, ENT( pPlayer->pev ) );
			WRITE_STRING( STRING( pev->noise ) );
		MESSAGE_END();

		// Read from the desired file
		int iLength, iCharCount = 0;
		char *pFileList;
		char *aFileList = pFileList = (char *)LOAD_FILE_FOR_ME( (char *)STRING( pev->message ), &iLength );

		// Send the message of the day, read it chunk-by-chunk and send it in parts
		while ( pFileList && *pFileList && iCharCount < MAX_MESSAGE_LENGTH )
		{
			char cChunk[MAX_MESSAGE_CHUNK + 1];

			if ( strlen( pFileList ) < MAX_MESSAGE_CHUNK )
			{
				strcpy( cChunk, pFileList );
			}
			else
			{
				strncpy( cChunk, pFileList, MAX_MESSAGE_CHUNK );
				cChunk[MAX_MESSAGE_CHUNK] = 0;	// strncpy doesn't always append the NULL terminator
			}

			iCharCount += strlen( cChunk );
			if ( iCharCount < MAX_MESSAGE_LENGTH )
				pFileList = aFileList + iCharCount; 
			else
				*pFileList = 0;

			MESSAGE_BEGIN( MSG_ONE, gmsgPaper, NULL, ENT( pPlayer->pev ) );
				WRITE_BYTE( *pFileList ? FALSE : TRUE );	// FALSE means there is still more message to come
				WRITE_STRING( cChunk );
			MESSAGE_END();
		}

		FREE_FILE( aFileList );
	}

	SUB_UseTargets( this, USE_TOGGLE, 0 );
}
