#include "LnQuery.h"

#include <VolumeRoster.h>
#include <Message.h>
#include <NodeMonitor.h>
#include <Entry.h>
#include <stdio.h>


// fetch 'old' index entries
int32 volume_query_thread( void * _data )
{
	VolumeQuery * query = (VolumeQuery*)_data;
	
	printf("  VolumeQuery(%s): Fetching\n", query->m_volume_name);
	
	BMessage reply;
	BMessage begin_fetch( VolumeQuery::BEGIN_FETCH );
	begin_fetch.AddString("volume", query->m_volume_name );
	
	query->m_target.SendMessage(&begin_fetch,&reply);
	
	query->m_query.Fetch();
	
	entry_ref ref;
	node_ref node_ref;
	
	for ( int32 c=0; query->m_query.GetNextRef(&ref) == B_OK; c++ )
	{
//		printf("  match %ld\n",c);
		BEntry entry( &ref );
		entry.GetNodeRef( &node_ref );
		
		BMessage msg(B_QUERY_UPDATE);
		msg.AddInt32("opcode",B_ENTRY_CREATED);
		msg.AddString("name", ref.name);
		msg.AddInt64("directory", ref.directory);
		msg.AddInt32("device", ref.device);
		msg.AddInt64("node", node_ref.node);
		
		if ( query->m_target.SendMessage( &msg ) != B_OK )
			break;
	}
	
	BMessage end_fetch( VolumeQuery::END_FETCH );
	end_fetch.AddString("volume", query->m_volume_name );
	
	query->m_target.SendMessage( &end_fetch, &reply );
	
	printf("  VolumeQuery(%s): Done fetching\n", query->m_volume_name);
	
	return 0;
}

// VOLUME QUERY

VolumeQuery::VolumeQuery( BMessenger msgr, BVolume vol, const char * predicate )
:	BLooper("VolumeQuery"),
	m_target( msgr ),
	m_volume( vol )
{
	m_volume.GetName(m_volume_name);
	printf("VolumeQuery: vol [%s]\n",m_volume_name);
	
	m_query.SetTarget( this );
	m_query.SetPredicate(predicate);
	
	if ( m_query.SetVolume( &m_volume ) != B_OK )
	{ // volume doesn't support queries, we can stop right here and now
		printf("  VolumeQuery(%s): Doesn't support queries\n", m_volume_name);
		delete this;
		return;
	} else
	{ // spawn thread
		Run(); // start looper
		
		// spawn fetcher thread
		m_thread = spawn_thread(
			volume_query_thread,
			"volume query",
			B_NORMAL_PRIORITY,
			this
		);
		resume_thread( m_thread );
	}
}

VolumeQuery::~VolumeQuery()
{
	kill_thread( m_thread );
}

void
VolumeQuery::MessageReceived( BMessage * msg )
{
	switch ( msg->what )
	{
		case VolumeQuery::BEGIN_FETCH:
		case VolumeQuery::END_FETCH:
		case B_QUERY_UPDATE:
			if ( m_target.SendMessage(msg) != B_OK )
			{
				printf("  VolumeQuery(%s): Exiting after messaging error\n", m_volume_name );
				BMessenger(this).SendMessage( B_QUIT_REQUESTED );
			}
			break;
		default:
			BLooper::MessageReceived(msg);
			break;
	}
}

// QUERY

LnQuery::LnQuery( BMessenger msgr, const char * predicate )
:	BLooper("Query"),
	m_target( msgr )
{
	Run();
	
	BVolumeRoster volumes;
	BVolume vol;
	
	for ( int c=0; volumes.GetNextVolume(&vol) == B_OK ; c++ )
	{ // spawn volume queries for all volumes
		if ( vol.KnowsQuery() )
			new VolumeQuery( BMessenger(this), vol, predicate );
	}
}

LnQuery::~LnQuery()
{
}

void
LnQuery::MessageReceived( BMessage * msg )
{
	switch ( msg->what )
	{
		case VolumeQuery::BEGIN_FETCH:
		case VolumeQuery::END_FETCH:
		case B_QUERY_UPDATE:
			if ( m_target.SendMessage(msg) != B_OK )
			{
				printf("  LnQuery exiting after messaging error\n");
				BMessenger(this).SendMessage( B_QUIT_REQUESTED );
			}
			break;
		default:
			BLooper::MessageReceived(msg);
			break;
	}
}

