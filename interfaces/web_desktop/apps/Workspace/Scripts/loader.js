/*©agpl*************************************************************************
*                                                                              *
* This file is part of FRIEND UNIFYING PLATFORM.                               *
*                                                                              *
* This program is free software: you can redistribute it and/or modify         *
* it under the terms of the GNU Affero General Public License as published by  *
* the Free Software Foundation, either version 3 of the License, or            *
* (at your option) any later version.                                          *
*                                                                              *
* This program is distributed in the hope that it will be useful,              *
* but WITHOUT ANY WARRANTY; without even the implied warranty of               *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                 *
* GNU Affero General Public License for more details.                          *
*                                                                              *
* You should have received a copy of the GNU Affero General Public License     *
* along with this program.  If not, see <http://www.gnu.org/licenses/>.        *
*                                                                              *
*****************************************************************************©*/
/** @file
 *
 * Tree application initial loader
 *
 * @author FL (Francois Lionet)
 * @date first pushed on 20/09/2017
 */

// This is the main run function for jsx files and FriendUP js apps
Application.run = function( msg )
{
	// Make a new window with some flags
	this.mainView = new View(
	{
		title: 'Friend Workspace',
		width: 1024,
		height: 768
	} );

	// Load a file from the same dir as the jsx file is located
	var self = this;
	var f = new File( 'Progdir:Templates/index.html' );
	f.onLoad = function( data )
	{
		// Set it as window content
		self.mainView.self = self.mainView;
		self.mainView.setContent( data );
	}
	f.load();

	// On closing the window, quit.
	this.mainView.onClose = function()
	{
		Application.quit();
	}
};

// Message handling
Application.receiveMessage = function( msg )
{
	switch( msg.command )
	{
	case 'quit':
		Application.quit();
		break;
	}
};
