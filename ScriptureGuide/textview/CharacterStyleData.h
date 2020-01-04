/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef CHARACTER_STYLE_DATA_H
#define CHARACTER_STYLE_DATA_H

#include <Font.h>
#include <Referenceable.h>


enum {
	STRIKE_OUT_NONE		= 0,
	STRIKE_OUT_SINGLE	= 1,
};


enum {
	UNDERLINE_NONE		= 0,
	UNDERLINE_SINGLE	= 1,
	UNDERLINE_DOUBLE	= 2,
	UNDERLINE_WIGGLE	= 3,
	UNDERLINE_DOTTED	= 4,
};

//need to adjust this :)
enum {
	NON_CHARATER_STYLES_CHANGED	= 0x00000000,
	FONT_FAMILY_CHANGED			= 0x00000001,
	FONT_FACE_CHANGED			= 0x00000002,
	FONT_STYLE_CHANGED			= 0x00000004,
	FONT_SIZE_CHANGED			= 0x00000008,
	FONT_SHEAR_CHANGED			= 0x00000010,
	FONT_ROTATION_CHANGED		= 0x00000020,
	
	FONT_SPACING_CHANGED		= 0x00000040,
	FONT_ENCODING_CHANGED		= 0x00000080,
	
	ITALIC_FACE_CHANGED			= 0x00000100,
	UNDERLINE_FACE_CHANGED		= 0x00000200,
	UNDERSCORE_FACE_CHANGED		= 0x00000400,
	NEGATIVE_FACE_CHANGED		= 0x00000800,
	OUTLINED_FACE_CHANGED		= 0x00001000,
	STRIKEOUT_FACE_CHANGED		= 0x00002000,
	BOLD_FACE_CHANGED			= 0x00004000,
	REGULAR_FACE_CHANGED		= 0x00008000,

	CONDENSED_FACE_CHANGED		= 0x00010000,
	LIGHT_FACE_CHANGED			= 0x00020000,
	HEAVY_FACE_CHANGED			= 0x00040000,
	FORE_GROUND_COLOR_CHANGED	= 0x00080000,
	BACKGROUND_COLOR_CHANGED	= 0x00100000,
	STRIKE_OUT_COLOR_CHANGED	= 0x00200000,
	UNDERLINE_COLOR_CHANGED		= 0x00400000,
	
	ASCENT_CHANGED				= 0x00800000,
	DESCENT_CHANGED				= 0x01000000,
	WIDTH_CHANGED				= 0x02000000,
	GLYPH_SPACING_CHANGED		= 0x04000000,
	ALL_CHARATER_STYLES_CHANGED	= 0xFFFFFFFF
};

class CharacterStyleData;
typedef BReference<CharacterStyleData>	CharacterStyleDataRef;


// You cannot modify a CharacterStyleData object once it has been
// created. Instead, the setter methods return a new object that equals
// the object on which the method has been called, except for the changed
// property. If the property already has the requested value, then a
// reference to the same object is returned.
class CharacterStyleData : public BReferenceable {
public:
								CharacterStyleData();
								CharacterStyleData(
									const CharacterStyleData& other);

			bool				operator==(
									const CharacterStyleData& other) const;
			bool				operator!=(
									const CharacterStyleData& other) const;

			CharacterStyleDataRef SetFont(const BFont& font);
	inline	const BFont&		Font() const
									{ return fFont; }

			CharacterStyleDataRef SetAscent(float ascent);

			// Returns the ascent of the configured font, unless the ascent
			// has been overridden by a fixed value with SetAscent().
			float				Ascent() const;

			CharacterStyleDataRef SetDescent(float descent);

			// Returns the descent of the configured font, unless the descent
			// has been overridden by a fixed value with SetDescent().
			float				Descent() const;

			CharacterStyleDataRef SetWidth(float width);
	inline	float				Width() const
									{ return fWidth; }

			CharacterStyleDataRef SetGlyphSpacing(float glyphSpacing);
	inline	float				GlyphSpacing() const
									{ return fGlyphSpacing; }

			CharacterStyleDataRef SetForegroundColor(color_which which);
			CharacterStyleDataRef SetForegroundColor(rgb_color color);
	inline	rgb_color			ForegroundColor() const
									{ return fFgColor; }
	inline	color_which			WhichForegroundColor() const
									{ return fWhichFgColor; }

			CharacterStyleDataRef SetBackgroundColor(color_which which);
			CharacterStyleDataRef SetBackgroundColor(rgb_color color);
	inline	rgb_color			BackgroundColor() const
									{ return fBgColor; }
	inline	color_which			WhichBackgroundColor() const
									{ return fWhichBgColor; }

			CharacterStyleDataRef SetStrikeOutColor(color_which which);
			CharacterStyleDataRef SetStrikeOutColor(rgb_color color);
	inline	rgb_color			StrikeOutColor() const
									{ return fStrikeOutColor; }
	inline	color_which			WhichStrikeOutColor() const
									{ return fWhichStrikeOutColor; }

			CharacterStyleDataRef SetUnderlineColor(color_which which);
			CharacterStyleDataRef SetUnderlineColor(rgb_color color);
	inline	rgb_color			UnderlineColor() const
									{ return fUnderlineColor; }
	inline	color_which			WhichUnderlineColor() const
									{ return fWhichUnderlineColor; }

			CharacterStyleDataRef SetStrikeOut(uint8 strikeOut);
	inline	uint8				StrikeOut() const
									{ return fStrikeOutStyle; }

			CharacterStyleDataRef SetUnderline(uint8 underline);
	inline	uint8				Underline() const
									{ return fUnderlineStyle; }


private:
			CharacterStyleData&	operator=(const CharacterStyleData& other);

private:
			BFont				fFont;

			// The following three values override glyph metrics unless -1
			// This is useful when you want to have a place-holder character
			// in the text. You would assign this character a CharacterStyle
			// which defines ascent, descent and width to fixed values, thereby
			// controlling the exact dimensions of the character, regardless of
			// font size. Instead of the character, one could render an icon
			// that is layouted within the text.
			float				fAscent;
			float				fDescent;
			float				fWidth;

			// Additional spacing to be applied between glyphs.
			float				fGlyphSpacing;

			color_which			fWhichFgColor;
			color_which			fWhichBgColor;

			color_which			fWhichStrikeOutColor;
			color_which			fWhichUnderlineColor;

			rgb_color			fFgColor;
			rgb_color			fBgColor;

			rgb_color			fStrikeOutColor;
			rgb_color			fUnderlineColor;

			uint8				fStrikeOutStyle;
			uint8				fUnderlineStyle;
			
			uint32				fChangedMask;
};


#endif // CHARACTER_STYLE_DATA_H
