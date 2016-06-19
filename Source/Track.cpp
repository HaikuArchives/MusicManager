#include "Track.h"

#include <Node.h>
#include <TypeConstants.h>
#include <stdlib.h>
#include <stdio.h>

Track::Track( entry_ref * entry )
:	m_track(0)
{
	m_entry = *entry;
	
	// load various attributes
	
	BNode node( &m_entry );
	
	node_ref n_ref;
	
	node.GetNodeRef( &n_ref );
	
	m_node = n_ref.node;
	
	// let's hope we don't need a bigger buffer
	// should read how large the attr's are, but that's too much work :P
	char		buf[4096];
	ssize_t		num_read;
	
	num_read = node.ReadAttr(
		"Audio:Artist",
		B_STRING_TYPE,
		0, // offset
		buf,
		sizeof(buf)
	);
	if ( num_read > 0 )
		m_artist = buf;
	else
		m_artist = "[unknown artist]";
	num_read = node.ReadAttr(
		"Audio:Album",
		B_STRING_TYPE,
		0, // offset
		buf,
		sizeof(buf)
	);
	if ( num_read > 0 )
		m_album = buf;
	else
		m_album = "[unknown album]";
	num_read = node.ReadAttr(
		"Audio:Track",
		B_INT32_TYPE,
		0, // offset
		&m_track,
		sizeof(m_track)
	);
	if ( num_read != sizeof(m_track) )
	{ // read track fail
		m_track = 0;
	}
	num_read = node.ReadAttr(
		"Audio:Title",
		B_STRING_TYPE,
		0, // offset
		buf,
		sizeof(buf)
	);
	if ( num_read > 0 )
		m_title = buf;
	else
		m_title = "[unknown title]";
	num_read = node.ReadAttr(
		"Audio:Length",
		B_STRING_TYPE,
		0, // offset
		buf,
		sizeof(buf)
	);
	if ( num_read > 0 )
	{
		int length_min = atoi( buf );
		int length_sec = atoi( &buf[3] );
		
		m_length = length_min * 60 + length_sec;
	} else
	{
		m_length = 0;
	}
}

Track::Track( const Track & track )
:	m_track(0)
{
	*this = track;
}

Track::~Track()
{
}

// copy

const Track &
Track::operator = ( const Track & track )
{
	m_entry = track.m_entry;
	m_node = track.m_node;
	
	m_artist = track.m_artist;
	m_album = track.m_album;
	m_track = track.m_track;
	m_title = track.m_title;
	m_length = track.m_length;
	
	return *this;
}

// get things

const entry_ref *
Track::GetEntry() const
{
	return &m_entry;
}

ino_t
Track::GetNode() const
{
	return m_node;
}

const char *
Track::GetArtist() const
{
	return m_artist.String();
}

const char *
Track::GetAlbum() const
{
	return m_album.String();
}

int32
Track::GetTrack() const
{
	return m_track;
}

const char *
Track::GetTitle() const
{
	return m_title.String();
}

int32
Track::GetLength() const
{
	return m_length;
}
