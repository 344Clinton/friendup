/*©mit**************************************************************************
*                                                                              *
* This file is part of FRIEND UNIFYING PLATFORM.                               *
* Copyright (c) Friend Software Labs AS. All rights reserved.                  *
*                                                                              *
* Licensed under the Source EULA. Please refer to the copy of the MIT License, *
* found in the file license_mit.txt.                                           *
*                                                                              *
*****************************************************************************©*/
/** @file
 *
 *  User Group Manager
 *
 * file contain all functitons related to user group management
 *
 *  @author PS (Pawel Stefanski)
 *  @date created 18/01/2019
 */

#include "user_group_manager.h"
#include "user_sessionmanager.h"

#include <system/systembase.h>
#include <util/sha256.h>
#include <system/fsys/device_handling.h>
#include <util/session_id.h>

/**
 * Create UserGroupManager
 *
 * @param sb pointer to SystemBase
 * @return UserGroupManager structure
 */
UserGroupManager *UGMNew( void *sb )
{
	UserGroupManager *sm;
	
	if( ( sm = FCalloc( 1, sizeof( UserGroupManager ) ) ) != NULL )
	{
		SystemBase *lsb = (SystemBase *)sb;
		sm->ugm_SB = sb;
		Log( FLOG_INFO,  "[SystemBase] Loading groups from DB\n");
	
		SQLLibrary *sqlLib = lsb->LibrarySQLGet( lsb );
		if( sqlLib != NULL )
		{
			int entries;
			sm->ugm_UserGroups = sqlLib->Load( sqlLib, UserGroupDesc, NULL, &entries );
			lsb->LibrarySQLDrop( lsb, sqlLib );
		}
		
		UserGroup *g = sm->ugm_UserGroups;
		while( g != NULL )
		{
			if( strcmp( g->ug_Name, "Admin" ) == 0 )
			{
				g->ug_IsAdmin = TRUE;
			}
			
			if( strcmp( g->ug_Name, "API" ) == 0 )
			{
				g->ug_IsAPI = TRUE;
			}
			g = (UserGroup *)g->node.mln_Succ;
		}
		
		pthread_mutex_init( &sm->ugm_Mutex, NULL );

		return sm;
	}
	return NULL;
}

/**
 * Delete UserManager
 *
 * @param um pointer to UserManager structure which will be deleted
 */

void UGMDelete( UserGroupManager *um )
{
	Log( FLOG_INFO,  "UGMDelete release groups\n");

	if( FRIEND_MUTEX_LOCK( &um->ugm_Mutex ) == 0 )
	{
		UserGroupDeleteAll( um->ugm_SB, um->ugm_UserGroups );

		um->ugm_UserGroups = NULL;
		FRIEND_MUTEX_UNLOCK( &um->ugm_Mutex );
	}
	pthread_mutex_destroy( &um->ugm_Mutex );
	
	FFree( um );
}

/**
 * Get UserGroup by ID
 *
 * @param um pointer to UserManager structure
 * @param id UserGroupID
 * @param return UserGroup structure if it exist, otherwise NULL
 */

UserGroup *UGMGetGroupByID( UserGroupManager *um, FULONG id )
{
	if( FRIEND_MUTEX_LOCK( &um->ugm_Mutex ) == 0 )
	{
		UserGroup *ug = um->ugm_UserGroups;
		while( ug != NULL )
		{
			if( ug->ug_ID == id )
			{
				FRIEND_MUTEX_UNLOCK( &um->ugm_Mutex );
				return ug;
			}
			ug = (UserGroup *)ug->node.mln_Succ;
		}
		FRIEND_MUTEX_UNLOCK( &um->ugm_Mutex );
	}
	return NULL;
}

/**
 * Get UserGroup by Name
 *
 * @param ugm pointer to UserManager structure
 * @param name name of the group
 * @param return UserGroup structure if it exist, otherwise NULL
 */

UserGroup *UGMGetGroupByName( UserGroupManager *ugm, const char *name )
{
	if( FRIEND_MUTEX_LOCK( &ugm->ugm_Mutex ) == 0 )
	{
		UserGroup *ug = ugm->ugm_UserGroups;
		while( ug != NULL )
		{
			if( ug->ug_Name != NULL && (strcmp( name, ug->ug_Name ) == 0 ) )
			{
				FRIEND_MUTEX_UNLOCK( &ugm->ugm_Mutex );
				return ug;
			}
			ug = (UserGroup *)ug->node.mln_Succ;
		}
		FRIEND_MUTEX_UNLOCK( &ugm->ugm_Mutex );
	}
	return NULL;
}

/**
 * Add UserGroup to list of groups
 *
 * @param ugm pointer to UserManager structure
 * @param ug pointer to new group which will be added to list
 * @param return 0 when success, otherwise error number
 */

int UGMAddGroup( UserGroupManager *ugm, UserGroup *ug )
{
	if( ugm == NULL )
	{
		FERROR("Cannot add NULL to group!\n");
		return 1;
	}
	if( FRIEND_MUTEX_LOCK( &ugm->ugm_Mutex ) == 0 )
	{
		ug->node.mln_Succ = (MinNode *) ugm->ugm_UserGroups;
		ugm->ugm_UserGroups = ug;
		FRIEND_MUTEX_UNLOCK( &ugm->ugm_Mutex );
	}
	return 0;
}

/**
 * Remove (diable) UserGroup on list of groups
 *
 * @param ugm pointer to UserManager structure
 * @param ug pointer to new group which will be disabled
 * @param return 0 when success, otherwise error number
 */

int UGMRemoveGroup( UserGroupManager *ugm, UserGroup *ug )
{
	if( FRIEND_MUTEX_LOCK( &ugm->ugm_Mutex ) == 0 )
	{
		ug->ug_Status = USER_GROUP_STATUS_DISABLED;
		FRIEND_MUTEX_UNLOCK( &ugm->ugm_Mutex );
	}
	return 0;
}

/**
 * Remove device from UserGroup
 *
 * @param sm pointer to UserManager structure
 * @param name name of the drive
 * @return pointer to removed drive when its avaiable, otherwise NULL
 */
File *UGMRemoveDrive( UserGroupManager *sm, const char *name )
{
	File *remdev = NULL;
	// device was not found on user device list
	UserGroup *ug = sm->ugm_UserGroups;
	
	if( FRIEND_MUTEX_LOCK( &sm->ugm_Mutex ) == 0 )
	{
		while( ug != NULL )
		{
			File *lf = ug->ug_MountedDevs;
			File *lastone = lf;
			while( lf != NULL )
			{
				DEBUG( "[UnMountFS] Checking fs in list %s == %s...\n", lf->f_Name, name );
				if( strcmp( lf->f_Name, name ) == 0 )
				{
					DEBUG( "[UnMountFS] Found one (%s == %s)\n", lf->f_Name, name );
					remdev = lf;
					break;
				}
				lastone = lf;
				lf = (File *)lf->node.mln_Succ;
			}
			
			// remove drive
		
			if( remdev != NULL )
			{
				if( remdev->f_Operations <= 0 )
				{
					DEBUG("[UserRemDeviceByName] Remove device from list\n");
				
					if( ug->ug_MountedDevs == remdev )		// checking if its our first entry
					{
						File *next = (File*)remdev->node.mln_Succ;
						ug->ug_MountedDevs = (File *)next;
						if( next != NULL )
						{
							next->node.mln_Pred = NULL;
						}
					}
					else
					{
						File *next = (File *)remdev->node.mln_Succ;
						//next->node.mln_Pred = (struct MinNode *)prev;
						if( lastone != NULL )
						{
							lastone->node.mln_Succ = (struct MinNode *)next;
						}
					}
				}
				else
				{
				//*error = FSys_Error_OpsInProgress;
				//return remdev;
				}
			}
			ug = (UserGroup *)ug->node.mln_Succ;
		}
		FRIEND_MUTEX_UNLOCK( &sm->ugm_Mutex );
	}
	return remdev;
}

/**
 * Mount all UserGroup drives
 *
 * @param sm pointer to UserManager structure
 * @return 0 when success, otherwise error number
 */
int UGMMountDrives( UserGroupManager *sm )
{
	SystemBase *sb = (SystemBase *)sm->ugm_SB;
	SQLLibrary *sqllib  = sb->LibrarySQLGet( sb );
	if( sqllib != NULL )
	{
		UserGroup *ug = sm->ugm_UserGroups;
		while( ug != NULL )
		{
			//UserGroupDeviceMount( l, sqllib, ug, NULL );
			UserGroupDeviceMount( sb, sqllib, ug, sb->sl_UM->um_APIUser );
			ug = (UserGroup *)ug->node.mln_Succ;
		}
		
		sb->LibrarySQLDrop( sb, sqllib );
	}
	return 0;
}

/**
 * Assign User to his groups in FC
 *
 * @param smgr pointer to UserGroupManager
 * @param usr pointer to user structure to which groups will be assigned
 * @return 0 when success, otherwise error number
 */

#define QUERY_SIZE 1024

int UGMAssignGroupToUser( UserGroupManager *smgr, User *usr )
{
	char *tmpQuery;
	DEBUG("[UMAssignGroupToUser] Assign group to user\n");

	//sprintf( tmpQuery, "SELECT UserGroupID FROM FUserToGroup WHERE UserID = '%lu'", usr->u_ID );
	if( smgr == NULL )
	{
		return 1;
	}
	
	tmpQuery = (char *)FCalloc( QUERY_SIZE, sizeof(char) );
	
	SystemBase *sb = (SystemBase *)smgr->ugm_SB;
	SQLLibrary *sqlLib = sb->LibrarySQLGet( sb );

	if( sqlLib != NULL )
	{
		sqlLib->SNPrintF( sqlLib, tmpQuery, QUERY_SIZE, "SELECT UserGroupID FROM FUserToGroup WHERE UserID = '%lu'", usr->u_ID );

		void *result = sqlLib->Query(  sqlLib, tmpQuery );
	
		if ( result == NULL ) 
		{
			FERROR("SQL query result is empty!\n");
			sb->LibrarySQLDrop( sb, sqlLib );
			return 2;
		}
		
		FBOOL isAdmin = FALSE;
		FBOOL isAPI = FALSE;

		char **row;
		int j = 0;
	
		if( usr->u_Groups )
		{
			FFree( usr->u_Groups );
			usr->u_Groups = NULL;
			usr->u_GroupsNr = 0;
		}
	
		int rows = sqlLib->NumberOfRows( sqlLib, result );
		if( rows > 0 )
		{
			usr->u_Groups = FCalloc( rows, sizeof( UserGroup *) );
		}
	
		DEBUG("[UMAssignGroupToUser] Memory for %d  groups allocated\n", rows );
	
		if( usr->u_Groups != NULL )
		{
			int pos = 0;
			usr->u_GroupsNr = rows;
		
			while( ( row = sqlLib->FetchRow( sqlLib, result ) ) )
			{
				DEBUG("[UMAssignGroupToUser] Going through loaded rows %d -> %s\n", j, row[ 0 ] );
				{
					char *end;
					FULONG gid = strtol( (char *)row[0], &end, 0 );
				
					DEBUG("[UMAssignGroupToUser] User is in group %lu\n", gid  );
				
					UserGroup *g = sb->sl_UGM->ugm_UserGroups;
					while( g != NULL )
					{
						if( g->ug_ID == gid )
						{
							if( g->ug_IsAdmin == TRUE )
							{
								isAdmin = g->ug_IsAdmin;
							}
							if( g->ug_IsAPI == TRUE )
							{
								isAPI = g->ug_IsAPI;
							}
							
							UserGroupAddUser( g, usr );
							DEBUG("[UMAssignGroupToUser] Added group %s to user %s\n", g->ug_Name, usr->u_Name );
							usr->u_Groups[ pos++ ] = g;
						}
						g  = (UserGroup *) g->node.mln_Succ;
					}
				}
			}
		}
		
		usr->u_IsAdmin = isAdmin;
		usr->u_IsAPI = isAPI;
	
		sqlLib->FreeResult( sqlLib, result );

		sb->LibrarySQLDrop( sb, sqlLib );
	}
	
	FFree( tmpQuery );
	
	return 0;
}

/**
 * Assign User to his groups in FC.
 * Groups are provided by string (comma is separator)
 *
 * @param um pointer to UserGroupManager
 * @param usr pointer to user structure to which groups will be assigned
 * @param groups groups provided as string, where comma is separator between group names
 * @return 0 when success, otherwise error number
 */
int UGMAssignGroupToUserByStringDB( UserGroupManager *um, User *usr, char *groups )
{
	if( groups == NULL )
	{
		FERROR("Group value is empty, cannot setup groups!\n");
		return 1;
	}
	char tmpQuery[ 512 ];
	
	DEBUG("[UMAssignGroupToUserByStringDB] Assign group to user start NEW GROUPS: %s\n", groups );
	
	SystemBase *sb = (SystemBase *)um->ugm_SB;
	SQLLibrary *sqlLib = sb->LibrarySQLGet( sb );
	
	if( sqlLib == NULL )
	{
		FERROR("Cannot get mysql.library slot\n");
		return -10;
	}
	
	// checking  how many groups were passed
	
	int i;
	int commas = 1;
	int glen = strlen( groups );
	
	if( groups != NULL )
	{
		for( i=0 ; i < glen; i++ )
		{
			if( groups[ i ] == ',' )
			{
				commas++;
			}
		}
	}
	
	//
	// put all groups into table
	
	char **ptr = NULL;
	if( ( ptr = FCalloc( commas, sizeof(char *) ) ) != NULL )
	{
		int pos = 0;
		ptr[ pos++ ] = groups;
		
		for( i=1 ; i < glen; i++ )
		{
			if( groups[ i ] == ',' )
			{
				groups[ i ] = 0;
				ptr[ pos++ ] = &(groups[ i+1 ]);
			}
		}
		
		//
		// going through all groups and create new "insert"
		//
		
		UserGroup **usrGroups = FCalloc( pos, sizeof( UserGroup *) );
		BufString *bs = BufStringNew();
		BufStringAdd( bs, "INSERT INTO FUserToGroup (UserID, UserGroupID ) VALUES ");
		
		FBOOL isAdmin = FALSE;
		FBOOL isAPI = FALSE;
		DEBUG("[UMAssignGroupToUserByStringDB] Memory for groups allocated\n");
		for( i = 0; i < pos; i++ )
		{
			UserGroup *gr = sb->sl_UGM->ugm_UserGroups;
			while( gr != NULL )
			{
				if( strcmp( gr->ug_Name, ptr[ i ] ) == 0 )
				{
					usrGroups[ i ] = gr;
					
					if( gr->ug_IsAdmin == TRUE )
					{
						isAdmin = TRUE;
					}
					
					if( gr->ug_IsAPI == TRUE )
					{
						isAPI = TRUE;
					}
					
					DEBUG("[UMAssignGroupToUserByStringDB] Group found %s will be added to user %s\n", gr->ug_Name, usr->u_Name );
					
					char loctmp[ 255 ];
					if( i == 0 )
					{
						snprintf( loctmp, sizeof( loctmp ),  "( %lu, %lu ) ", usr->u_ID, gr->ug_ID ); 
					}
					else
					{
						snprintf( loctmp, sizeof( loctmp ),  ",( %lu, %lu ) ", usr->u_ID, gr->ug_ID ); 
					}
					BufStringAdd( bs, loctmp );
					break;
				}
				gr = (UserGroup *) gr->node.mln_Succ;
			}
		}
		
		usr->u_IsAdmin = isAdmin;
		usr->u_IsAPI = isAPI;
		
		// removeing old group conections from DB
		
		snprintf( tmpQuery, sizeof(tmpQuery), "DELETE FROM FUserToGroup WHERE UserID = %lu AND UserGroupId IN ( SELECT ID FROM FUserGroup fug WHERE fug.Type = 'Level')", usr->u_ID ) ;

		if( sqlLib->QueryWithoutResults( sqlLib, tmpQuery ) !=  0 )
		{
			FERROR("Cannot call query: '%s'\n", tmpQuery );
		}

		if( sqlLib->QueryWithoutResults( sqlLib, bs->bs_Buffer  ) !=  0 )
		{
			FERROR("Cannot call query: '%s'\n", bs->bs_Buffer );
		}

		if( usr->u_Groups != NULL )
		{
			FFree( usr->u_Groups );
		}
		usr->u_Groups = usrGroups;
		
		if( bs != NULL )
		{
			BufStringDelete( bs );
		}
		
		FFree( ptr );
	}
	sb->LibrarySQLDrop( sb, sqlLib );
	DEBUG("[UMAssignGroupToUserByStringDB] Assign  groups to user end\n");
	
	return 0;
}
