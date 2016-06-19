#ifndef TRACK_H
#define TRACK_H

#include <String.h>
#include <Entry.h>

class Track
{
	private:
		entry_ref	m_entry;
		ino_t		m_node;
		
		BString		m_artist;
		BString		m_album;
		int32		m_track;
		BString		m_title;
		int32		m_length;
		
	public:
		Track( entry_ref * );
		Track( const Track & );
		~Track();
		
		const Track & operator = ( const Track & );
		
		const entry_ref *	GetEntry() const;
		ino_t				GetNode() const;
		
		const char *	GetArtist() const;
		const char *	GetAlbum() const;
		int32			GetTrack() const;
		const char *	GetTitle() const;
		int32			GetLength() const;
};

#endif

