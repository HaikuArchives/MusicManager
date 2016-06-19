#ifndef BETTER_LIST_ITEM_H
#define BETTER_LIST_ITEM_H

#include <ListItem.h>

#include <string>

class BetterListItem : public BListItem
{
	public:
		BetterListItem();
		virtual ~BetterListItem();
		
		virtual bool operator < ( const BetterListItem & ) const = 0;
};


class BetterStringItem : public BetterListItem
{
	private:
		string	m_string;
		
	public:
		BetterStringItem( const char * );
		virtual ~BetterStringItem();
		
		virtual bool operator < ( const BetterListItem & ) const;
		
		virtual void DrawItem( BView *, BRect, bool );
		
		virtual const char * GetText() const;
};


#endif
