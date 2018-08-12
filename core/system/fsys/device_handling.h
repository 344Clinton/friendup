/*©mit**************************************************************************
*                                                                              *
* This file is part of FRIEND UNIFYING PLATFORM.                               *
* Copyright 2014-2017 Friend Software Labs AS                                  *
*                                                                              *
* Permission is hereby granted, free of charge, to any person obtaining a copy *
* of this software and associated documentation files (the "Software"), to     *
* deal in the Software without restriction, including without limitation the   *
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or  *
* sell copies of the Software, and to permit persons to whom the Software is   *
* furnished to do so, subject to the following conditions:                     *
*                                                                              *
* The above copyright notice and this permission notice shall be included in   *
* all copies or substantial portions of the Software.                          *
*                                                                              *
* This program is distributed in the hope that it will be useful,              *
* but WITHOUT ANY WARRANTY; without even the implied warranty of               *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                 *
* MIT License for more details.                                                *
*                                                                              *
*****************************************************************************©*/
/** @file device_handling.h
 *  Device handling header
 *
 *  @author PS (Pawel Stefanski)
 *  @date 2015
 */

#ifndef __SYSTEM_FSYS_DEVICE_HANDLING_H__
#define __SYSTEM_FSYS_DEVICE_HANDLING_H__

#include <core/types.h>
#include <core/library.h>
#include <system/systembase.h>

int RescanHandlers( SystemBase *l );

//
//
//

int RescanDOSDrivers( SystemBase *l );

//
//
//

int UnMountFS( struct SystemBase *l, struct TagItem *tl, UserSession *usr );

//
//
//

int MountFS( struct SystemBase *l, struct TagItem *tl, File **mfile, User *usr );

//
//
//

int UserGroupDeviceMount( SystemBase *l, SQLLibrary *sqllib, UserGroup *usrgrp, User *usr );

//
//
//

int MountFSNoUser( struct SystemBase *l, struct TagItem *tl, File **mfile );

//
//
//

File *GetFileByPath( User *usr, char **dstpath, const char *path );

//
//
//

int DeviceMountDB( SystemBase *l, File *rootDev, FBOOL mount );

//
//
//

File *GetUserDeviceByUserID( SystemBase *l, SQLLibrary *sqllib, FULONG uid, const char *devname );

//
//
//

void UserNotifyFSEvent( struct SystemBase *b, char *evt, char *path );

//
//
//

void UserNotifyFSEvent2( SystemBase *sb, User *u, char *evt, char *path );

//
//
//

int MountDoorByRow( SystemBase *l, User *usr, char **row, User *mountUser );

//
//
//

int CheckAndMountWorkgroupDrive( SystemBase *l, char *type, User *usr, FUQUAD id, int mounted );

//
//
//

int RefreshUserDrives( SystemBase *l, User *u, BufString *bs );

//
//
//

int DeviceRelease( SystemBase *l, File *rootDev );

//
//
//

int DeviceUnMount( SystemBase *l, File *rootDev, User *usr );

//
// find comma and return position
//

static inline int ColonPosition( const char *c )
{
	int res = 0;
	
	for( unsigned int i=0 ; i < strlen( c ) ; i++ )
	{
		if( c[ i ] == ':' )
		{
			return i;
		}
	}
	
	return res;
}

//
//
//

File *GetRootDeviceByName( User *usr, char *devname );

#endif // __SYSTEM_FSYS_DEVICE_HANDLING_H__
