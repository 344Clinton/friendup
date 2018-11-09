/*©agpl*************************************************************************
*                                                                              *
* This file is part of FRIEND UNIFYING PLATFORM.                               *
* Copyright (c) Friend Software Labs AS. All rights reserved.                  *
*                                                                              *
* Licensed under the Source EULA. Please refer to the copy of the GNU Affero   *
* General Public License, found in the file license_agpl.txt.                  *
*                                                                              *
*****************************************************************************©*/
/** @file
 *
 * Build web application Friend options
 * First application using FriendUI
 *
 * @author FL (Francois Lionet)
 * @date first pushed on 19/10/2017
 */

Application.run = function()
{
	this.iconPath = 'Progdir:Gfx/waIcon.png';
	this.previewPath = 'Progdir:Gfx/waPreview.png';
	this.types =
	[
		'Internet',
		'Games',
		'Office',
		'Graphics',
		'Programming'
	];
};
function changeWebAppIcon()
{
	var self = this;
	var description =
	{
		triggerFunction: function( item )
		{
			if ( item )
			{
				self.iconPath = item[ 0 ].Path;
				document.getElementById( 'waIcon' ).src = getImageUrl( item[ 0 ].Path );
			}
		},
		path: "Mountlist:",
		type: "load",
		title: i18n( 'i18n_waChangeWebAppIconTitle' ),
		filename: ""
	}
	var d = new Filedialog( description );
};
function changeWebAppPreview()
{
	var self = this;
	var description =
	{
		triggerFunction: function( item )
		{
			if ( item )
			{
				self.previewPath = item[ 0 ].Path;
				document.getElementById( 'waPreview' ).src = getImageUrl( item[ 0 ].Path );
			}
		},
		path: "Mountlist:",
		type: "load",
		title: i18n( 'i18n_waChangeWebAppPreviewTitle' ),
		filename: ""
	}
	var d = new Filedialog( description );
};
function createProject()
{
	var self = Application;

	// Open a path file dialog and return selected files
	var name = document.getElementById( 'waName' ).value;
	var description =
	{
		triggerFunction: function( path )
		{
			if ( typeof path === 'string' && path != '')
			{
				// Creates a directory to save the files
				if ( path.substring( path.length - 1 ) == ':' )
				{
					Alert( i18n( 'i18n_waAlert' ), i18n( 'i18n_waNeedAFolder' ) );
					return;
				}
				path += '/';

				var shell = new Shell();
				var count = 0;
				shell.onReady = function()
				{
					shell.execute( 'makedir "' + path + '"' );
				}
				shell.onPipe = function( msg )
				{
					if ( msg.data == true )
					{
						count++;
						switch( count )
						{
							case 1:
								shell.execute( 'makedir "' + path + 'Resources"' );
								break;
							case 2:
								shell.execute( 'makedir "' + path + 'Scripts"');
								break;
							case 3:
								shell.execute( 'makedir "' + path + 'Templates"');
								break;
							case 4:
								saveFiles( path, function( response ) {} );
								break;
						}
					}
				}
			}
		},
		path: "Mountlist:",
		type: "save",
		title: i18n( 'i18n_waFileSelectorTitle' ),
		filename: name
	}
	var d = new Filedialog( description );
};
function saveFiles( path, callback )
{
	var self = Application;
	var f = new File( 'Progdir:Scripts/websiteTemplate.js' );
	f.onLoad = function( data )
	{
		var template = data;

		// Gets values from dialog box
		var url = document.getElementById( 'waUrl' ).value;
		var name = document.getElementById( 'waName' ).value;
		var width = document.getElementById( 'waWidth' ).value;
		var height = document.getElementById( 'waHeight' ).value;
		var type  = document.getElementById( 'waType' ).value;
		var author  = document.getElementById( 'waAuthor' ).value;
		var description  = document.getElementById( 'waDescription' ).value;
		var email  = document.getElementById( 'waEmail' ).value;
		if ( author == '' )
			author = i18n( 'i18n_blankAuthor' );
		if ( description == '' )
			description = i18n( 'i18n_blankDescription' );
		if ( email == '' )
			email = i18n( 'i18n_blankEmail' );

		// Checks values
		if ( url.substring( 0, 7 ) != 'http://' && url.substring( 0, 8 ) != 'https://' )
		{
			Alert( i18n( 'i18n_waAlert' ), i18n( 'i18n_waWrongUrl' ) );
			return callback( 'Error' );
		}
		if ( name.length == 0 )
		{
			Alert( i18n( 'i18n_waAlert' ), i18n( 'i18n_waWrongName' ) );
			return callback( 'Error' );
		}
		var flag = true;
		if ( width.length > 0 )
		{
			if ( parseInt( width, 10 ) < 32 )
				flag = false;
		}
		if ( !flag )
		{
			Alert( i18n( 'i18n_waAlert' ), i18n( 'i18n_waWrongWidth' ) );
			return callback( 'Error' );
		}
		if ( height.length > 0 )
		{
			if ( parseInt( height, 10 ) < 32 )
				flag = false;
		}
		if ( !flag )
		{
			Alert( i18n( 'i18n_waAlert' ), i18n( 'i18n_waWrongHeight' ) );
			return callback( 'Error' );
		}

		// Replace strings in template
		var about0 = i18n( 'i18n_waAbout0' );
		var about1 = i18n( 'i18n_waAbout1' );
		var aboutBlank = i18n( 'i18n_aboutBlank' );
		var close = i18n( 'i18n_waClose' );
		var file = i18n( 'i18n_waFile' );
		template = replaceString( template, 'title_holder', name );
		template = replaceString( template, 'url_holder', url );
		template = replaceString( template, 'width_holder', width );
		template = replaceString( template, 'height_holder', height );
		template = replaceString( template, 'category_holder', self.types[ type ] );
		template = replaceString( template, 'author_holder', author );
		template = replaceString( template, 'about0_holder', about0 );
		template = replaceString( template, 'about1_holder', about1 );
		template = replaceString( template, 'about2_holder', author );
		template = replaceString( template, 'close_holder', close );
		template = replaceString( template, 'description_holder', description );
		template = replaceString( template, 'email_holder', email );
		template = replaceString( template, 'file_holder', file );

		// Cut and save files
		var cut = cutString( template, '<CUTMAIN>', '<ENDCUTMAIN>' );
		var fl = new File( path + name + '.jsx' );
		fl.save( cut );
		cut = cutString( template, '<CUTINDEX>', '<ENDCUTINDEX>' );
		var fl = new File( path + 'Templates/index.html' );
		fl.save( cut );
		cut = cutString( template, '<CUTABOUT>', '<ENDCUTABOUT>' );
		var fl = new File( path + 'Templates/about.html' );
		fl.save( cut );

		// Copy the icon
		var fi = new File( self.iconPath );
		fi.onLoad = function( iData )
		{
			var fSave = new File( path + 'Resources/icon.png' );
			fSave.save( iData, null, 'wb' );

			// Copy the preview
			var fp = new File( self.previewPath );
			fp.onLoad = function( pData )
			{
				var pSave = new File( path + 'Resources/preview.png' );
				pSave.save( pData, null, 'wb' );
				setTimeout( function()
				{
					var files = 
					[
						name + '.jsx',
						'Templates/index.html',
						'Templates/about.html',
						'Resources/icon.png',
						'Resources/preview.png'
					];
					var projectFiles = [];
					var count = files.length;
					for ( var f = 0; f < files.length; f++ )
					{
						getFileInformation( files[ f ], path, function( object )
						{
							projectFiles.push( object );
							count--;
							if ( count == 0 )
							{
								var project = 
								{
									ProjectName: name,
									Author:      author,
									Version:     '1.0',
									Category:    self.types[ type ],
									Description: description,
									Permissions: [],
									Files: projectFiles,      
									Screenshots: [],
									Libraries:   [], 
								};
								Application.sendMessage( { command: 'project_loadfromweb', data: project, filename: path + name + '.apf' } );
							}
						} );
					}

					// Get the information on the files of the project
					function getFileInformation( file, path, callback )
					{
						var doorPath = path;
						var pos = file.indexOf( '/' );
						if ( pos >= 0 )
						{
							doorPath += file.substring( 0, pos + 1 );
							file = file.substring( pos + 1 );
						}
						var door = new Door( doorPath );
						door.getIcons( function( objects )
						{
							for ( var o = 0; o < objects.length; o++ )
							{
								if ( objects[ o ].Filename == file )
								{
									objects[ o ].Path = objects[ o ].Path.substring( path.length );
									return callback( objects[ o ] );
								}
							}
						} );
					}
				}, 1000 );
			}
			fp.load( 'rb' );
		}
		fi.load( 'rb' );
	}
	f.load();
};
function replaceString( template, search, replace )
{
	var pos = template.indexOf( search );
	while( pos >= 0 )
	{
		template = template.substring( 0, pos ) + replace + template.substring( pos + search.length );
		pos = template.indexOf( search );
	}
	return template;
};
function cutString( template, start, end )
{
	var pStart = template.indexOf( start );
	if ( pStart >= 0)
	{
		var pEnd = template.indexOf( end );
		if ( pEnd >= 0 )
		{
			return template.substring( pStart + start.length, pEnd );
		}
	}
	return false;
};
function close( msg )
{
	Application.sendMessage( { command: 'fromwebsite_closewin' } );
};

