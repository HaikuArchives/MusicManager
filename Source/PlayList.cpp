#include "PlayList.h"

#include <Message.h>
#include <stdio.h>
#include <Entry.h>
#include <Node.h>
#include <NodeInfo.h>
#include <Mime.h>
#include <Roster.h>
#include <stdlib.h>
#include <Button.h>
#include <PopUpMenu.h>
#include <MenuItem.h>
#include <CheckBox.h>
#include <Path.h>
#include <StringView.h>

#include "TrackItem.h"

#define SOUNDPLAY_SIGNATURE "application/x-vnd.marcone-soundplay"
#define TRACKER_SIGNATURE "application/x-vnd.Be-TRAK"

BMessenger
find_soundplay_messenger()
{
	// first look for a replicant in Tracker
	BMessenger msgr( TRACKER_SIGNATURE );
	BMessage msg( B_GET_PROPERTY ), reply;
	int32 error=0;
	
	int32 num_windows = 0;
	
	msg = BMessage( B_COUNT_PROPERTIES );
	msg.AddSpecifier( "Window" );
	if ( msgr.SendMessage( &msg, &reply ) != B_OK )
		;
	if ( reply.FindInt32("result",&num_windows) != B_OK )
		;
	
	printf("Windows: %ld\n", num_windows);
	for ( int32 window=0; window<num_windows; window++ )
	{
		#define WAIT_UNTIL 100000
		int32 num_view1 = 0;
		
		msg = BMessage( B_COUNT_PROPERTIES );
		msg.AddSpecifier( "View" );
		msg.AddSpecifier( "Window", window );
		if ( msgr.SendMessage( &msg, &reply, WAIT_UNTIL, WAIT_UNTIL ) != B_OK )
			continue;
		if ( reply.FindInt32("result",&num_view1) != B_OK )
			continue;
		
		printf("Views in window %ld: %ld\n", window, num_view1 );
		for ( int32 view1=0; view1<num_view1; view1++ )
		{
			int32 num_view2 = 0;
			msg = BMessage( B_COUNT_PROPERTIES );
			msg.AddSpecifier( "View" );
			msg.AddSpecifier( "View", view1 );
			msg.AddSpecifier( "Window", window );
			if ( msgr.SendMessage( &msg, &reply ) != B_OK )
				;
			if ( reply.FindInt32("result",&num_view2) != B_OK )
				;
		
			printf("Views in view %ld of window %ld: %ld\n", view1, window, num_view2 );
			for ( int32 view2=0; view2 < num_view2; view2++ )
			{
				msg = BMessage( B_GET_PROPERTY );
				msg.AddSpecifier("InternalName");
				msg.AddSpecifier("View",view2);
				msg.AddSpecifier("View",view1);
				msg.AddSpecifier("Window",window);
				msgr.SendMessage(&msg, &reply);
				reply.FindInt32("error",&error);
				reply.PrintToStream();
				if ( reply.what != B_REPLY || error != 0 )
					break;
				
				const char * name = reply.FindString("result");
				if ( name )
				{
					printf("Found possible match: %s\n", name);
					if ( strcmp(name,"SoundPlay replicant") == 0 )
					{
						msg = BMessage( B_GET_PROPERTY );
						msg.AddSpecifier("Messenger");
						msg.AddSpecifier("View",view2);
						msg.AddSpecifier("View",view1);
						msg.AddSpecifier("Window",window);
						msgr.SendMessage(&msg, &reply);
						reply.FindInt32("error",&error);
						if ( error )
							break;
					
						BMessenger result;
						if ( reply.FindMessenger("result",&result) == B_OK )
							return result;
					}
				}
			}
		}
	}
	
	// no replicant found, start SoundPlay and get a track from it

	if ( !be_roster->IsRunning(SOUNDPLAY_SIGNATURE) )
	{ // soundplay not running, start it
		be_roster->Launch(SOUNDPLAY_SIGNATURE);
	}
	
	msgr = BMessenger(SOUNDPLAY_SIGNATURE);
	
	// create track
	BMessage create_track_msg( B_CREATE_PROPERTY );
	create_track_msg.AddSpecifier( "track" );
	
	msgr.SendMessage( &create_track_msg, &reply );
	
	BMessenger reply_msgr;
	reply.FindMessenger("messenger",&reply_msgr);
	
	return reply_msgr;
}

class ListView : public BListView
{
	private:
		BPopUpMenu	m_menu;
		bool		m_dragging;
		int			m_drag_from;
		int			m_drag_to;
		
	public:
		enum {
			REMOVE = 'PLre',
			CROP = 'PLcr'
		};
	
		ListView( BRect rect, uint32 follow )
		:	BListView( rect, "list", B_MULTIPLE_SELECTION_LIST, follow ),
			m_menu("",false,false),
			m_dragging( false ),
			m_drag_from( 0 ),
			m_drag_to( 0 )
		{
			m_menu.AddItem( new BMenuItem("Remove", new BMessage(REMOVE)) );
			m_menu.AddItem( new BMenuItem("Crop", new BMessage(CROP)) );
		};
		
		virtual ~ListView()
		{
		};
		
		virtual void MouseDown( BPoint pos )
		{
			int32 buttons = 0;
			Window()->CurrentMessage()->FindInt32("buttons",&buttons);
			
			if ( buttons & B_SECONDARY_MOUSE_BUTTON )
			{ // rmb press
				pos.x -= 2;
				pos.y -= 2;
				
				m_menu.SetTargetForItems( this );
				
				BPoint screen_point = ConvertToScreen(pos);
				BRect open_rect( screen_point, screen_point );
				open_rect.InsetBy(-3,-3);
				
				m_menu.Go(
					screen_point,
					true,
					true,
					open_rect,
					true
				);
			} else
				BListView::MouseDown(pos);
		};
		
		virtual void MouseMoved( BPoint pos, uint32 transit, const BMessage * msg )
		{
			BListView::MouseMoved( pos, transit, msg );
			
			if ( !msg || !msg->FindString("LnPlayListDrag") )
				return;
			
			switch ( transit )
			{
				case B_EXITED_VIEW:
					Invalidate(); // Why doesn't this work, eh?
				// fall through
				case B_OUTSIDE_VIEW:
					return;
				default:
					break;
			}
			
			// redraw old drag target
			Invalidate( ItemFrame( m_drag_to ) );
			
			// update drag target
			m_drag_to = IndexOf( pos );
			if ( m_drag_to < 0 )
			{ // drag below bottom of list
				m_drag_to = CountItems();
			} else
			{
				BRect target_frame = ItemFrame( m_drag_to );
			
				if ( pos.y > target_frame.top + target_frame.Height()/2 )
				{ // lower half of target item, target++
					m_drag_to++;
				}
			}
			
			// update new drag target
			Invalidate( ItemFrame( m_drag_to ) );
		};
		
		virtual void Draw( BRect update_rect )
		{
			BListView::Draw(update_rect);
			
			if ( m_dragging )
			{ // display line where item will be inserted
				SetHighColor( 255,0,0 );

				if ( m_drag_to < CountItems() )
				{
					BRect frame = ItemFrame( m_drag_to );
					StrokeLine( frame.LeftTop(), frame.RightTop() );
				} else
				{
					BRect frame = ItemFrame( CountItems()-1 );
					StrokeLine( frame.LeftBottom(), frame.RightBottom() );
				}
			}
		};
		
		virtual void MouseUp( BPoint where )
		{
			if ( m_dragging && Bounds().Contains(where) )
			{ // drop item at current position
				if ( m_drag_from < m_drag_to )
					m_drag_to--;
				
				MoveItem( m_drag_from, m_drag_to );
				
				Invalidate();
			}
			
			m_dragging = false;
			
			BListView::MouseUp( where );
		};
		
		virtual bool InitiateDrag( BPoint point, int32 index, bool wasSelected )
		{
			// don't drag more than one song
			if ( CurrentSelection(1) >= 0 )
				return false;
			
			// initiate drag
			BMessage dragmsg('DATA');
			dragmsg.AddString("LnPlayListDrag","yup");
			dragmsg.AddPointer("LnSourceList", this);
			// add songs to message
			for ( int i=0; CurrentSelection(i) >= 0; i++ )
			{
				TrackItem * item = dynamic_cast<TrackItem*>(ItemAt(CurrentSelection(i)));
				dragmsg.AddRef("refs", item->GetTrack()->GetEntry());
			}
			
			BRect rect(0,0,40,20);
			rect.OffsetTo( point.x-rect.Width()/2, point.y-rect.Height()/2 );
			DragMessage(&dragmsg,rect,this);
			
			m_dragging = true;
			m_drag_from = index;
			m_drag_to = index;
			
			SetMouseEventMask(B_POINTER_EVENTS,B_LOCK_WINDOW_FOCUS);
			
			return true;
		};
		
		virtual void KeyDown( const char * key, int32 num )
		{
			switch ( *key )
			{
				case B_DELETE:
				{
					BMessenger msgr(this);
					msgr.SendMessage( REMOVE );
				}	break;
				default:
					BListView::KeyDown(key,num);
			}
		};
		
		virtual void MessageReceived( BMessage * msg )
		{
			switch ( msg->what )
			{
				case REMOVE:
				{
					while ( CurrentSelection(0) >= 0 )
					{
						BListItem * item = ItemAt( CurrentSelection(0) );
						RemoveItem( item );
						delete item;
					}
					
					BMessenger msgr(Window());
					msgr.SendMessage(PlayList::UPDATE_TOTAL_LENGTH);
				}	break;
				
				case CROP:
				{ // remove unselected items
					for ( int c=0; ItemAt(c); c++ )
					{
						for (; ItemAt(c) && !ItemAt(c)->IsSelected(); )
						{
							BListItem * item = ItemAt( c );
							RemoveItem( item );
							delete item;
						}
					}
					
					BMessenger msgr(Window());
					msgr.SendMessage(PlayList::UPDATE_TOTAL_LENGTH);
				}	break;
		
				case 'DATA':
				{ // file drop / load confirmed
					ListView * sourceList = NULL;
					if ( msg->FindPointer("LnSourceList", (void**)&sourceList) == B_OK )
					{
						// don't add files to self
						if ( sourceList == this )
							return;
					}
					
					entry_ref ref;
			
					bool drop_in_empty_list = ( CountItems() == 0 );
			
					for ( int c=0; msg->FindRef("refs",c,&ref)==B_OK; c++ )
					{
						// make sure it's an audio file
						BNode node(&ref);
						BNodeInfo info(&node);
				
						char mimetype[512];
						info.GetType( mimetype );
				
						BMimeType full_mime( mimetype );
						BMimeType super_type;
						full_mime.GetSupertype( &super_type );
				
						if ( strcmp(mimetype,"text/x-playlist") == 0 )
						{ // it's a playlist
							BPath path(&ref);
					
							FILE * file = fopen(path.Path(), "r");
					
							// read lines, convert to items, add to list
							while ( !feof(file) )
							{
								char filename[512];
								if ( !fgets(filename,sizeof(filename),file) )
									break;
						
								if ( filename[strlen(filename)-1] == '\n' )
									filename[strlen(filename)-1] = 0; // remove end-of-line
						
								if ( strlen(filename) == 0 )
									continue;
						
								BEntry entry(filename);
						
								if ( !entry.Exists() )
								{
									printf("File doesn't exist: %s!\n",filename);
									continue;
								}
						
								entry_ref file_ref;
								entry.GetRef( &file_ref );
						
								// trust items in playlist to be audio files
								TrackItem * item = new TrackItem( new Track(&file_ref) );
						
								AddItem( item );
//								printf("File: %s\n",filename);
							}
							
							fclose(file);
						}
				
						if ( super_type == "audio" )
						{ // it's an audio file, add to list
							printf("audio file drop: %s\n", ref.name );
					
							TrackItem * item = new TrackItem( new Track(&ref) );
					
							AddItem( item );
						}
					}
			
					BMessenger msgr(Window());
					msgr.SendMessage(PlayList::UPDATE_TOTAL_LENGTH);
				}	break;
		
				default:
					BListView::MessageReceived(msg);
			}
		}
};

// checks if soundplay has stopped playing
int32
player_checker_thread( void * _data )
{
	printf("player_checker_thread running\n");
	PlayerCheckerData * data = (PlayerCheckerData*)_data;
	
	data->msg = BMessage(B_GET_PROPERTY);
	data->msg.AddSpecifier("pitch");
	
	data->snooze_time = 100000;
	
	int32 zero_count = 0;
	
	while ( true )
	{
		snooze( data->snooze_time );
		
		data->reply.MakeEmpty();
		if ( data->player.SendMessage( &data->msg, &data->reply ) != B_OK )
		{
			// lower poll-rate if app not running
			data->snooze_time = 1000000;
			continue;
		}
		
		// set poll rate to 10/sec
		data->snooze_time = 100000;
		
		data->reply.FindFloat("result", &data->res );
		
		if ( data->res == 0 )
		{
			if ( ++zero_count > 3 )
			{
				data->owner.SendMessage( &data->stop_message );
				zero_count = 0;
			}
		}
	}
}

PlayList::PlayList()
:	BWindow(
		BRect(430,50,900,480),
		"Playlist",
		B_TITLED_WINDOW,
		B_ASYNCHRONOUS_CONTROLS|B_QUIT_ON_WINDOW_CLOSE
	),
	m_total_length( 0 ),
	m_playing( false ),
	m_last_index(-1)
{
//	SettingsFile settings;
//	settings.Load();
	
	BRect window_frame;
/*	if ( settings.FindRect("playlist_frame",&window_frame) == B_OK )
	{
		MoveTo(window_frame.LeftTop());
		ResizeTo(window_frame.Width(), window_frame.Height());
	}
*/	
	// info
	BRect info_rect = Bounds();
	info_rect.bottom = 49;
	
	m_info = new TrackInfoView(info_rect);
	AddChild( m_info );
	
	// list
	BRect list_rect = Bounds();
	
	list_rect.top = 50;
	list_rect.bottom = 184;
	list_rect.InsetBy(2,2);
	list_rect.right -= B_V_SCROLL_BAR_WIDTH + 1;
	
	m_list = new ListView( list_rect, B_FOLLOW_ALL );
	
	AddChild( m_scroller = new BScrollView(
		"scroller", 
		m_list,
		B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP,
		0,
		false,
		true
	));
	
	m_list->SetInvocationMessage( new BMessage(LIST_INVOKE) );
	
	// 'Next song selected randomly from below if upper list is empty'
	BStringView * str = new BStringView(
		BRect(0,list_rect.bottom+3, Bounds().right, list_rect.bottom+16), "next song message", 
		"Next song selected randomly from below if upper list is empty",
		B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP
	);
	str->SetViewColor( ui_color(B_PANEL_BACKGROUND_COLOR) );
	AddChild(str);
	
	// random list
	list_rect = Bounds();
	
	list_rect.top = 200;
	list_rect.bottom -= 29;
	list_rect.InsetBy(2,2);
	list_rect.right -= B_V_SCROLL_BAR_WIDTH + 1;
	
	m_random_list = new ListView( list_rect, B_FOLLOW_ALL );
	
	AddChild( m_random_scroller = new BScrollView(
		"scroller", 
		m_random_list,
		B_FOLLOW_ALL,
		0,
		false,
		true
	));
	
	m_random_list->SetInvocationMessage( new BMessage(LIST_INVOKE) );
	m_random_list->MakeFocus(true);
	
	// buttons
	BRect buttons_rect = Bounds();
	buttons_rect.top = buttons_rect.bottom - 29;
	
	BView * button_holder = new BView(
		buttons_rect, "", B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM, B_WILL_DRAW
	);
	button_holder->SetViewColor( ui_color(B_PANEL_BACKGROUND_COLOR) );
	AddChild(button_holder);
	
	button_holder->AddChild(
		new BButton( BRect(2,2,80,27), "start_stop", "Start", new BMessage(START_STOP) )
	);
	
	button_holder->AddChild(
		new BButton( BRect(85,2,130,27), "load", "Load", new BMessage(LOAD_REQUEST) )
	);
	
	button_holder->AddChild(
		new BButton( BRect(135,2,180,27), "save", "Save", new BMessage(SAVE_REQUEST) )
	);
	
	// load & save panels
	m_save_panel = new BFilePanel( B_SAVE_PANEL );
		m_save_panel->SetMessage( new BMessage( SAVE_CONFIRM ) );
		m_save_panel->SetTarget( this );
	m_load_panel = new BFilePanel( B_OPEN_PANEL );
		m_load_panel->SetMessage( new BMessage( LOAD_CONFIRM ) );
		m_load_panel->SetTarget( this );
	
	// player checker thread
	m_thread_data.player = find_soundplay_messenger();
	m_thread_data.owner = BMessenger( this );
	m_thread_data.stop_message = BMessage(PLAYER_STOPPED);
	
	m_thread = spawn_thread(
		player_checker_thread,
		"player checker",
		B_NORMAL_PRIORITY,
		&m_thread_data
	);
	resume_thread( m_thread );
	

	UpdateTotalLength();
	
}

PlayList::~PlayList()
{
	BRect temp;
	
/*	SettingsFile settings;
	settings.Load();
	
	if ( settings.FindRect("playlist_frame", &temp) == B_OK )
		settings.ReplaceRect("playlist_frame", Frame() );
	else
		settings.AddRect("playlist_frame", Frame() );
	
	settings.Save();
*/	
	// free list items
	for ( int32 c=0; c<m_random_list->CountItems(); c++ )
		delete m_random_list->ItemAt(c);
	
	delete m_load_panel;
	delete m_save_panel;
	
	kill_thread( m_thread );
}

void
PlayList::MessageReceived( BMessage * msg )
{
	switch ( msg->what )
	{
		case UPDATE_TOTAL_LENGTH:
			UpdateTotalLength();
			break;
		
		case START_STOP:
		{ // Start/Stop button pressed
			m_playing = !m_playing;
			
			BButton * button = dynamic_cast<BButton*>(FindView("start_stop"));
			
			if ( m_playing )
				button->SetLabel("Stop");
			else
				button->SetLabel("Start");
			
			BMessage msg( B_SET_PROPERTY ), reply;
			msg.AddSpecifier( "pitch" );
			if ( m_playing )
			{ // tell SoundPlay to start playing
				msg.AddFloat( "data", 1.0 );
			} else
			{ // tell SoundPlay to stop playing
				msg.AddFloat( "data", 0.0 );
			}
			
			m_thread_data.player.SendMessage( &msg, &reply );
			
			BMessenger(this).SendMessage( PLAYER_STOPPED ); // open file
		}	break;
		
		// open load/save panel
		case LOAD_REQUEST:
			m_load_panel->Show();
			break;
		case SAVE_REQUEST:
			m_save_panel->Show();
			break;
		
		// perform save
		case SAVE_CONFIRM:
		{
			entry_ref dir_ref;
			if ( msg->FindRef("directory", &dir_ref) != B_OK )
			{ // damaged message
				break;
			}
			
			BDirectory directory( &dir_ref );
			
			const char * filename;
			
			if ( msg->FindString("name", &filename) != B_OK )
			{ // damaged message
				break;
			}
			
			// now we have the full path, let's create a file!
			BFile file;
			directory.CreateFile(filename,&file);
			
			if ( file.InitCheck() != B_OK )
			{
				printf("Error opening file when saving\n");
				break;
			}
			
			// set filetype
			BNodeInfo fileinfo( &file );
			fileinfo.SetType("text/x-playlist");
			
			// Ok, let's start saving..
			for ( int c=0; c<m_random_list->CountItems(); c++ )
			{
				TrackItem * item = dynamic_cast<TrackItem*>(m_random_list->ItemAt(c));
				
				const Track * track = item->GetTrack();
				
				const entry_ref * entry = track->GetEntry();
				
				BPath path( entry );
				
				const char * full_path = path.Path();
				
				file.Write(full_path, strlen(full_path) );
				file.Write("\n",1);
			}
			// done
		}	break;
		
		case NEXT:
		case PLAYER_STOPPED:
		{
			if ( (m_random_list->CountItems() > 0) && m_playing )
			{ // select next song to play
				TrackItem * item = NULL;
				bool usedTopList = false;
				
				if ( m_list->CountItems() > 0 )
				{
					usedTopList = true;
					item = dynamic_cast<TrackItem*>(m_list->ItemAt(0));
					m_list->RemoveItem(item);
				}
				
				if ( !item )
				{ // no item in top list, select one at random
					int32 index = m_last_index;
					
					do {
						index = random() % m_random_list->CountItems();
					} while ( m_random_list->CountItems() > 1 && index == m_last_index );
					
					item = dynamic_cast<TrackItem*>(m_random_list->ItemAt(index));
					
					m_last_index = index;
				}
				
				if ( item )
				{
					PlayTrack( item->GetTrack() );
					
					if ( usedTopList )
					{
						delete item;
					}
				}
			}
		}	break;
		
		case LIST_INVOKE:
		{ // double click in list, play selected file
			TrackItem * item = dynamic_cast<TrackItem*>(m_random_list->ItemAt(m_random_list->CurrentSelection()));
			
			if ( item )
			{
				m_last_index = m_random_list->IndexOf(item);
				PlayTrack( item->GetTrack() );
			}
		}	break;
		
//		case LOAD_CONFIRM:
		case PLAY_FILE:
		{
			entry_ref file;
			
			if ( msg->FindRef("file", &file) == B_OK )
			{
				PlayFile( &file );
			}
		}	break;
		
		default:
			BWindow::MessageReceived(msg);
	}
}

void
PlayList::PlayFile( const entry_ref * ref )
{
	BMessage msg(B_SET_PROPERTY), reply;
	msg.AddSpecifier("file");
	msg.AddRef( "data", ref );
	if ( m_thread_data.player.SendMessage( &msg, &reply ) != B_OK )
	{ // if play file fails, try once to get a fresh messenger and try again
		m_thread_data.player = find_soundplay_messenger();
		
		if ( m_thread_data.player.SendMessage( &msg, &reply ) != B_OK )
			; // should be: throw "Error connecting to SoundPlay!";
	}
}

void
PlayList::PlayTrack( const Track * track )
{
	m_info->SetTrack( track );
	
	PlayFile( track->GetEntry() );
}

void
PlayList::UpdateTotalLength()
{
	m_total_length = 0;
	for ( int c=0; c<m_random_list->CountItems(); c++ )
	{
		TrackItem * item = dynamic_cast<TrackItem*>( m_random_list->ItemAt(c) );
		
		m_total_length += item->GetTrack()->GetLength();
	}
	
	char text[512];
	if ( m_total_length >= 60*60 )
	{ // one hour or more
		sprintf(text, "%ld tracks in random list, %ld:%02ld:%02ld",
			m_random_list->CountItems(),
			m_total_length/3600,
			(m_total_length % 3600)/60,
			m_total_length % 60
		);
	} else
	{
		sprintf(text, "%ld tracks in random list, %ld:%02ld",
			m_random_list->CountItems(),
			m_total_length/60,
			m_total_length%60
		);
	}
	m_info->SetStaticInfo( text );
	m_info->Invalidate();
}
