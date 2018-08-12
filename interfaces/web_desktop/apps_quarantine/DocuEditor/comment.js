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

Application.run = function( msg, iface )
{
}

Application.receiveMessage = function( msg )
{
	if( !msg.command ) return;
	
	if( msg.command == 'init' )
	{
		ge( 'Comment' ).innerHTML = msg.subject;
		ge( 'ID' ).value = msg.id;
	}
}

function saveComment()
{
	var m = new Module( 'friendreference' );
	m.onExecuted = function( e, d )
	{
		Application.sendMessage( {
			topicid: ge( 'ID' ).value,
			command: 'getcomments'
		} );
		setTimeout( function()
		{
			cancelComment();
		}, 150 );
	}
	m.execute( 'comment', {
		topicid: ge( 'ID' ).value,
		comment: ge( 'CommentContent' ).value
	} );
}

// Just close view
function cancelComment()
{
	Application.sendMessage( { method: 'close', type: 'view' } );
}

