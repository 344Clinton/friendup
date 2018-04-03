<?php
/*©lgpl*************************************************************************
*                                                                              *
* This file is part of FRIEND UNIFYING PLATFORM.                               *
*                                                                              *
* This program is free software: you can redistribute it and/or modify         *
* it under the terms of the GNU Lesser General Public License as published by  *
* the Free Software Foundation, either version 3 of the License, or            *
* (at your option) any later version.                                          *
*                                                                              *
* This program is distributed in the hope that it will be useful,              *
* but WITHOUT ANY WARRANTY; without even the implied warranty of               *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                 *
* GNU Affero General Public License for more details.                          *
*                                                                              *
* You should have received a copy of the GNU Lesser General Public License     *
* along with this program.  If not, see <http://www.gnu.org/licenses/>.        *
*                                                                              *
*****************************************************************************©*/

//TK-820
function InstallApp( $user_id, $app_name )
{
	global $Logger;
	
	if( $path = FindAppInSearchPaths( $app_name ) )
	{
		if( file_exists( $path . '/Config.conf' ) )
		{
			$f = file_get_contents( $path . '/Config.conf' );
			
			// Path is dynamic!
			$f = preg_replace( '/\"Path[^,]*?\,/i', '"Path": "' . $path . '/",', $f );

			// Store application!
			$a = new dbIO( 'FApplication' );
			$a->Config = $f;
			$a->UserID = $user_id;
			$a->Name = $app_name;
			$a->Permissions = 'UGO';
			$a->DateInstalled = date( 'Y-m-d H:i:s' );
			$a->DateModified = $a->DateInstalled;
			$a->Save();
			if( $a->ID > 0 )
			{
				$Logger->log( 'App installation OK, id is ' . $a->ID );
			}
		}
	}
}

// Check workgroup specific expansion
// Load user's workgroups
if( $wgroups = $SqlDatabase->FetchObjects( '
	SELECT ug.Name FROM FUserGroup ug, FUserToGroup ugu, FUser u
	WHERE
		ug.Type = \'Workgroup\' AND u.ID = \'' . $User->ID . '\' AND 
		ugu.UserID = u.ID AND ug.ID = ugu.UserGroupID
' ) )
{
	// Check each workgroup
	foreach( $wgroups as $wgroup )
	{
		$wkey = strtolower( str_replace( ' ', '_', $wgroup->Name ) );
		// Is the script already run?
		$s = new dbIO( 'FSetting' );
		$s->Type = 'system';
		$s->Key = 'firstlogin_' . $wkey;
		$s->UserID = $User->ID;
		if( !$s->Load() )
		{
			// Load custom script for this workgroup
			if( file_exists( 'cfg/firstlogin_' . $wkey . '.php' ) )
			{
				$Logger->log( 'Found workgroup ' . $wkey . ' firstlogin script.' );
				require( 'cfg/firstlogin_' . $wkey . '.php' );
				// The script has run, register it
				$s->Save();
			}
		}
	}
}

// Prevent wizard for user
if( $Config->preventwizard == 1 )
{
	$s = new dbIO( 'FSetting' );
	$s->UserID = $User->ID;
	$s->Type = 'system';
	$s->Key = 'wizardrun';
	$s->Load();
	$s->Data = '1';
	$s->Save();
}

// Check that it really is first login
$s = new dbIO( 'FSetting' );
$s->Type = 'system';
$s->Key = 'firstlogin';
$s->UserID = $User->ID;
if( !$s->Load() )
{
	// Check for expansion
	if( file_exists( 'cfg/firstlogin.php' ) )
	{
		require( 'cfg/firstlogin.php' );
	}
	// Do defaults
	else
	{
		require( 'firstlogin.defaults.php' );
	}
	// Now we had first login!
	$s->Save();
}

