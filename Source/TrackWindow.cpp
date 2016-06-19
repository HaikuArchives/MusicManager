#include "TrackWindow.h"

#include "TrackItem.h"
#include "PlayList.h"
#include "LnQuery.h"

#include <Query.h>
#include <NodeMonitor.h>
#include <Roster.h>
#include <stdio.h>

BetterListItem *
each_adder( BetterListItem * _item, void * data)
{
	BMessage * msg = (BMessage*)data;
	TrackItem * item = dynamic_cast<TrackItem*>(_item);
	
	if ( item )
	{
		msg->AddRef("refs",item->GetTrack()->GetEntry() );
	}
	
	return NULL;
}


class TrackListView : public BetterListView
{
	public:
		TrackListView( BRect rect )
		:	BetterListView( rect, "list", B_MULTIPLE_SELECTION_LIST, B_FOLLOW_ALL )
		{
		};
		
		virtual ~TrackListView()
		{
		};
		
		virtual bool InitiateDrag(BPoint pos, int32, bool)
		{
			BMessage msg('DATA');
			
			for ( int c=0; CurrentSelection(c); c++ )
			{
				TrackItem * item = dynamic_cast<TrackItem*>( CurrentSelection(c) );
				if ( item ) 
				{ // item is a TrackItem
					msg.AddRef("refs",item->GetTrack()->GetEntry() );
				} else 
				{ // item is an album or an artist, find out which and add all below
					EachItemUnder( CurrentSelection(c), false, each_adder, &msg);
				}
			}
			
			BRect dragrect(-20,-5,20,5);
			dragrect.OffsetTo( pos );
			
			DragMessage(&msg, dragrect );
			
			return true;
		};
};

TrackWindow::TrackWindow()
:	BWindow(
		BRect(50,50,400,600),
		"Tracks",
		B_TITLED_WINDOW,
		B_QUIT_ON_WINDOW_CLOSE|B_ASYNCHRONOUS_CONTROLS
	),
	m_fetching(0)
{
//	SettingsFile settings;
//	settings.Load();
	
	BRect window_frame;
/*	if ( settings.FindRect("tracklist_frame",&window_frame) == B_OK )
	{
		MoveTo(window_frame.LeftTop());
		ResizeTo(window_frame.Width(), window_frame.Height());
	}
*/	
	// info
	BRect info_rect = Bounds();
	info_rect.bottom = 29;
	
	m_info = new TrackListInfo( info_rect );
	AddChild( m_info );
	
	// list
	BRect list_rect = Bounds();
	list_rect.top = 30;
	list_rect.InsetBy(2,2);
	list_rect.right -= B_V_SCROLL_BAR_WIDTH;
	
	m_list = new TrackListView( list_rect );
	m_list->SetMessage( new BMessage(LIST_INVOKE) );
	
	AddChild( m_scroller = new BScrollView(
		"scroller",
		m_list,
		B_FOLLOW_ALL,
		0,
		false,
		true
	) );
	
	m_list->MakeFocus(true);
}

TrackWindow::~TrackWindow()
{
	BRect temp;
	
/*	SettingsFile settings;
	settings.Load();
	
	if ( settings.FindRect("tracklist_frame", &temp) == B_OK )
		settings.ReplaceRect("tracklist_frame", Frame() );
	else
		settings.AddRect("tracklist_frame", Frame() );
	
	settings.Save();
*/	
	// free list items

	// add stuff here
}

void
TrackWindow::MessageReceived( BMessage * msg )
{
	switch (msg->what)
	{
		case VolumeQuery::BEGIN_FETCH:
		{
			m_fetching++;
		}	break;
		case VolumeQuery::END_FETCH:
		{
			m_fetching--;
			
			if ( !m_fetching )
			{ // no more fetches underway, add to list
				//printf("sorting tracks..\n");
				
				// m_fetched_items.SortItems( compare_track_items );

				//printf("adding tracks..\n");
				
				m_list->SupressUpdates(true);
				
				for ( int i=0; i<m_fetched_items.CountItems(); i++ )
				{ // add items to list
					TrackItem * item = (TrackItem*)m_fetched_items.ItemAt(i);
					
					if ( !item )
					{
						//printf("Something fishy..\n");
						continue;
					}
					
					AddTrack( item, false );
				}
				
				SortTracks();
				
				m_list->SupressUpdates(false);
			}
		}	break;
		case LIST_INVOKE:
		{
			TrackItem * item = dynamic_cast<TrackItem*>(m_list->CurrentSelection());
			
			if ( item )
			{
				// send message to playlist window!
				// DON'T -- be_roster->Launch( item->GetTrack()->GetEntry() );
				BMessage playmsg( PlayList::PLAY_FILE );
				playmsg.AddRef( "file", item->GetTrack()->GetEntry() );
				
				m_playlist_messenger.SendMessage( &playmsg );
			} else {
				BetterListItem * i = m_list->CurrentSelection();
				
				if ( i->IsExpanded() )
					m_list->Collapse( i );
				else
					m_list->Expand( i );
			}
		}	break;
		case B_QUERY_UPDATE:
		{
			int32 op = 0;
			int64 node = 0;
			msg->FindInt32("opcode",&op);
			msg->FindInt64("node",&node);
			
			switch ( op )
			{
				case B_ENTRY_CREATED:
				{
					const char	* name;
					dev_t		device;
					ino_t		directory;
					ino_t		node;
					
					msg->FindString("name", &name );
					msg->FindInt64("directory", &directory);
					msg->FindInt32("device", &device);
					msg->FindInt64("node", &node);
					
					bool found = false;
					for ( int c=0; m_list->ItemAt(c); c++ )
					{
						TrackItem * item = dynamic_cast<TrackItem*>(m_list->ItemAt(c));
						
						if ( item && item->IsNode(node) )
						{
							found = true;
							break;
						}
					}
					
					if ( !found )
					{ // a new file is added
						entry_ref ref( device, directory, name );
						
						TrackItem * item = new TrackItem( new Track(&ref), false );
						
						m_info->TrackAdded( item->GetTrack() );
						
						if ( m_fetching )
						{
							m_fetched_items.AddItem( item );
						} else
						{
							AddTrack( item );
							SortTracks();
						}
					}
				}	break;
				case B_ENTRY_REMOVED:
				{
					for ( int c=0; m_list->ItemAt(c); c++ )
					{
						TrackItem * item = dynamic_cast<TrackItem*>(m_list->ItemAt(c));
						
						if ( item && item->IsNode(node) )
						{
							m_info->TrackRemoved( item->GetTrack() );
							
							BetterListItem * album = m_list->Superitem(item);
							
							m_list->RemoveItem( item );
							
							delete item;
							
							// remove album if empty
							if ( m_list->CountItemsUnder(album,true) == 0 )
							{
								BetterListItem * artist = m_list->Superitem(album);
								
								m_list->RemoveItem( album );
								
								delete album;
								
								// remove artist if empty
								if ( m_list->CountItemsUnder(artist,true) == 0 )
								{
									m_list->RemoveItem( artist );
									
									delete artist;
								}
							}
							
							// if last track in album and/or last album in artist
							// delete these as well
							break;
						}
					}
					
				}	break;
				default:
					break;
			}
		}	break;
		default:
			BWindow::MessageReceived(msg);
	}
}

void
TrackWindow::SetPlaylistMessenger( BMessenger msgr )
{
	m_playlist_messenger = msgr;
}

void
TrackWindow::AddTrack( TrackItem * item, bool should_sort )
{
	// find artist
					
	string artist_string = item->GetTrack()->GetArtist();
					
	map<string,Artist*>::iterator i;
					
	i = m_artist_map.find( artist_string );
	
	Artist * artist = NULL;
	
	if ( i == m_artist_map.end() )
	{ // artist not found, create!
		artist = new Artist;
		artist->name = artist_string;
		artist->listitem = new BetterStringItem( artist_string.c_str() );
		artist->listitem->SetExpanded(false);
		m_list->AddItem( artist->listitem );
		m_artist_map[artist_string] = artist;
	} else 
	{ // artist found
		artist = i->second;
	}
	
	// find album
					
	string album_string = item->GetTrack()->GetAlbum();
	
	map<string,BetterListItem*>::iterator j;
	
	j = artist->albums.find( album_string );
					
	BetterListItem * album = NULL;
	
	if ( j == artist->albums.end() )
	{ // album not found
		album = new BetterStringItem( album_string.c_str() );
		album->SetExpanded(false);
		artist->albums[album_string] = album;
		m_list->AddUnder(album, artist->listitem);
	} else
	{ // album found
		album = j->second;
	}
	
	// add track under album
	
	m_list->AddUnder( item, album );
}


void
TrackWindow::SortTracks()
{
	m_list->SortItemsUnder(NULL,false);
}
