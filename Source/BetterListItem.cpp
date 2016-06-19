#include "BetterListItem.h"

#include <stdio.h>

#include <View.h>

BetterListItem::BetterListItem()
:	BListItem()
{
}

BetterListItem::~BetterListItem()
{
}

BetterStringItem::BetterStringItem( const char * str )
{
	m_string = str;
}

BetterStringItem::~BetterStringItem()
{
}

bool
BetterStringItem::operator < (const BetterListItem & _b ) const
{
	const BetterStringItem * b = dynamic_cast<const BetterStringItem*>(&_b);
	
	if ( !b )
	{
		return &_b < this;
	}
	
//	printf("BetterStringItem::operator <: %s, %s\n", m_string.c_str(), b->m_string.c_str() );
	return strcasecmp(m_string.c_str(), b->m_string.c_str()) < 0;
}

void
BetterStringItem::DrawItem( BView * owner, BRect frame, bool draw_everything )
{
	rgb_color background;
	rgb_color foreground;
	
	if ( IsSelected() )
	{
		background = (rgb_color){181,203,247,255};
		foreground = (rgb_color){0,0,0,255};
	} else
	{
		background = owner->ViewColor();
		foreground = (rgb_color){0,0,0,255};
	}
	
	// draw background
	owner->SetHighColor( background );
	owner->FillRect( frame );
	
	// draw text
	owner->SetLowColor( background );
	owner->SetHighColor( foreground );
	
	owner->DrawString( m_string.c_str(), BPoint(frame.left+4, frame.bottom-2) );
}

const char *
BetterStringItem::GetText() const
{
	return m_string.c_str();
}
