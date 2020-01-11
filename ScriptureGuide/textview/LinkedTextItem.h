#ifndef LINKED_TEXT_ITEM_H
#define LINKED_TEXT_ITEM_H

#include <String.h>

//try to build this as a kind of double linked List

class LinkedTextItem {
public:
								LinkedTextItem();
								LinkedTextItem(LinkedTextItem &newPrev,
									LinkedTextItem &newNext, int32 newLength=0);
								LinkedTextItem(const LinkedTextItem& other);
			//if i get destroed try to link the prev and next Item together??
			virtual				~LinkedTextItem();


			LinkedTextItem&		operator=(const LinkedTextItem& other);
			bool				operator==(const LinkedTextItem& other) const;
			bool				operator!=(const LinkedTextItem& other) const;
			/*TODO
			 *later we should add < > for easy comparison
			 * and +,- and +=  -= for easy forward and backward in the list
			 */
			void				SetPrev(LinkedTextItem &newPrev){prev=&newPrev;};
			LinkedTextItem*		Prev(){return prev;};
			void				SetNext(LinkedTextItem &newNext){next=&newNext;};
			LinkedTextItem*		Next(){return next;};
			//returns the end of the previous LinkedTextItem
			//so its always up to date
			int32				Start();
	
			//returns the Length of the Object
			int32				Lengt() {return length;};
			
	virtual	BString*			Text();
			
			//returns the Start + Length()
			int32				End();

			int32				Index();

private:
			LinkedTextItem	*prev;
			LinkedTextItem	*next;
			int32			length;
};


#endif // LINKED_TEXT_ITEM_H
