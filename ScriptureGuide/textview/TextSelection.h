/*
 * Copyright 2014, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef TEXT_SELECTION_H
#define TEXT_SELECTION_H


#include <SupportDefs.h>
#include <algorithm>
#include <stdlib.h>

class TextSelection {
public:
								TextSelection();
								TextSelection(int32 anchor, int32 caret);
								TextSelection(const TextSelection& other);

			TextSelection&		operator=(const TextSelection& other);
			bool				operator==(const TextSelection& other) const;
			bool				operator!=(const TextSelection& other) const;

			void				SetAnchor(int32 anchor);
	inline	int32				Anchor() const
									{ return fAnchor; }

			void				SetCaret(int32 caret);
	inline	int32				Caret() const
									{ return fCaret; }

	inline	int32				Start() const
									{return std::min(fAnchor,fCaret);}
	inline	int32				End() const
									{return std::max(fAnchor,fCaret);}
	inline	int32				Length() const
									{return std::abs(fAnchor-fCaret);}

private:
			int32				fAnchor;
			int32				fCaret;
};


#endif // TEXT_SELECTION_H
