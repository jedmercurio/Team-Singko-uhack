////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_GRAPHICS_h
#define _MITOV_GRAPHICS_h

#include <Mitov.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>

namespace Mitov
{
	class GraphicsElementBasic;
	class GraphicsTextElementBasic;
	class Graphics;
//---------------------------------------------------------------------------
	enum TArduinoTextHorizontalAlign { thaLeft, thaCenter, thaRight };
//---------------------------------------------------------------------------
	enum TArduinoTextVerticalAlign { tvaTop, tvaCenter, tvaBottom };
//---------------------------------------------------------------------------
	class GraphicsIntf
	{
	protected:
		Mitov::SimpleList<GraphicsElementBasic *>	FElements;

	public:
		virtual void GetPosition( int32_t &AX, int32_t &AY ) { AX = 0; AY = 0; }
		virtual void RegisterRender( GraphicsElementBasic *AItem ) 
		{
			FElements.push_back( AItem );
		}

		virtual Graphics *GetGraphics() = 0;
	};
//---------------------------------------------------------------------------
	class GraphicsTextIntf
	{
	protected:
		Mitov::SimpleList<GraphicsTextElementBasic *>	FElements;

	public:
		virtual void RegisterRender( GraphicsTextElementBasic *AItem ) 
		{
			FElements.push_back( AItem );
		}

	};
//---------------------------------------------------------------------------
	enum TArduinoMonochromeColor { tmcBlack, tmcWhite, tmcInvert, tmcNone };
//---------------------------------------------------------------------------
	class GraphicsElementBasic : public OpenWire::Object
	{
	protected:
		GraphicsIntf	&FOwner;

	public:
		virtual void Render( bool AForce ) = 0;

	public:
		GraphicsElementBasic( GraphicsIntf &AOwner ) :
			FOwner( AOwner )
		{
			FOwner.RegisterRender( this );
		}
	};
//---------------------------------------------------------------------------
	class GraphicsTextElementBasic
	{
	public:
		virtual void Enter( Graphics *AGraphics ) {}
		virtual void Leave( Graphics *AGraphics ) {}

	public:
		GraphicsTextElementBasic( GraphicsTextIntf &AOwner )
		{
			AOwner.RegisterRender( this );
		}
	};
//---------------------------------------------------------------------------
	class GraphicsElementClocked : public GraphicsElementBasic, public Mitov::ClockingSupport
	{
		typedef GraphicsElementBasic inherited;

	public:
		virtual void DoClockReceive( void *_Data ) override
		{
			Render( true );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class Graphics : public Print /*Adafruit_GFX*/, public GraphicsIntf
	{
//		typedef Adafruit_GFX inherited;
		typedef Print inherited;

	protected:
		const unsigned char * FFont;

	protected:
		const int16_t
			WIDTH, HEIGHT;   // This is the 'raw' display w/h - never changes

		int16_t
			_width, _height, // Display w/h as modified by current rotation
			cursor_x, cursor_y;

		uint16_t
			textcolor, textbgcolor;

		TArduinoMonochromeColor	BacgroundColor = tmcBlack;

		uint8_t
			textsize,
			rotation;

		bool wrap : 1;   // If set, 'wrap' text at right edge of display
		bool _cp437 : 1; // If set, use correct CP437 charset (default is off)

	public:
		void PrintChar( char AValue )
		{
			write( AValue );
		}

		void PrintChar( byte AValue )
		{
			write( AValue );
		}

	public:
		virtual Graphics *GetGraphics() override { return this; }
		virtual void Scroll( int32_t X, int32_t Y, uint16_t color ) = 0;

	public:
		// This MUST be defined by the subclass:
		virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;

	public:
		void setFont( const unsigned char *AFont )
		{
			FFont = AFont;
		}

		const unsigned char *getFont()
		{
			return FFont;
		}

		void getTextColor( uint16_t &AColor, uint16_t &ABackgroundColor )
		{
			AColor = textcolor;
			ABackgroundColor = textbgcolor; 
		}

		void drawEllipse(int16_t xc, int16_t yc, int16_t rx, int16_t ry, uint16_t color) 
		{
			drawPixel( xc, yc-ry, color);
			drawPixel( xc, yc+ry, color);

			drawEllipseHelper( xc, yc, rx, ry, 0xF, color );
		}

		void drawEllipseHelper(int16_t xc, int16_t yc, int16_t rx, int16_t ry, uint8_t ACorners, uint16_t color) 
		{
			//Region 1
			int16_t p = ry*ry-rx*rx*ry+rx*rx/4;
			int16_t x=0;
			int16_t y=ry;

			while(2.0*ry*ry*x <= 2.0*rx*rx*y)
			{
				if(p < 0)
				{
					x++;
					p = p+2*ry*ry*x+ry*ry;
				}

				else
				{
					x++;y--;
					p = p+2*ry*ry*x-2*rx*rx*y-ry*ry;
				}

				if( ACorners & 4 )
					drawPixel(xc+x,yc+y,color);

				if( ACorners & 2 )
					if( y != 0 || ACorners != 0xF )
						drawPixel(xc+x,yc-y,color);

				if( ACorners & 8 )
					drawPixel(xc-x,yc+y,color);

				if( ACorners & 1 )
					if( y != 0 || ACorners != 0xF )
						drawPixel(xc-x,yc-y,color);

			}

			//Region 2
			p = ry*ry*(x+0.5)*(x+0.5)+rx*rx*(y-1)*(y-1)-rx*rx*ry*ry;
			while(y > 0)
			{
				if(p <= 0)
				{
					x++;
					y--;
					p = p+2*ry*ry*x-2*rx*rx*y+rx*rx;
				}

				else
				{
					y--;
					p = p-2*rx*rx*y+rx*rx;
				}

				if( ACorners & 4 )
					drawPixel(xc+x,yc+y,color);

				if( ACorners & 2 )
					if( y != 0 || ACorners != 0xF )
						drawPixel(xc+x,yc-y,color);

				if( ACorners & 8 )
					drawPixel(xc-x,yc+y,color);

				if( ACorners & 1 )
					if( y != 0 || ACorners != 0xF )
						drawPixel(xc-x,yc-y,color);
			}
		}

		void fillEllipse(int16_t x0, int16_t y0, int16_t rx, int16_t ry, uint16_t color) 
		{
//			drawFastVLine(x0, y0-r, 2*r+1, color);
			fillEllipseHelper( x0, y0, rx, ry, 0xF, color );
		}

		void fillEllipseHelper(int16_t xc, int16_t yc, int16_t rx, int16_t ry, uint8_t ACorners, uint16_t color) 
		{
			//Region 1
			int16_t p = ry*ry-rx*rx*ry+rx*rx/4;
			int16_t x=0;
			int16_t y=ry;

			int16_t ALastY = -10;

			while(2.0*ry*ry*x <= 2.0*rx*rx*y)
			{
				if(p < 0)
				{
					x++;
					p = p+2*ry*ry*x+ry*ry;
				}

				else
				{
					x++;y--;
					p = p+2*ry*ry*x-2*rx*rx*y-ry*ry;
				}

				if( y != ALastY )
				{
					ALastY = y;
					if( ACorners == 0xF )
					{
						drawFastHLine( xc-x, yc+y, 2 * x, color );

						if( y != 0 )
							drawFastHLine( xc-x, yc-y, 2 * x, color );
					}

					else
					{
						if( ACorners & 8 )
							drawFastHLine( xc-x, yc+y, x, color );

						if( ACorners & 4 )
							drawFastHLine( xc, yc+y, x, color );

						if( ACorners & 1 )
							drawFastHLine( xc-x, yc-y, x, color );

						if( ACorners & 2 )
							drawFastHLine( xc, yc-y, x, color );
					}
				}
			}


			//Region 2
			p = ry*ry*(x+0.5)*(x+0.5)+rx*rx*(y-1)*(y-1)-rx*rx*ry*ry;
			while(y > 0)
			{
				if(p <= 0)
				{
					x++;
					y--;
					p = p+2*ry*ry*x-2*rx*rx*y+rx*rx;
				}

				else
				{
					y--;
					p = p-2*rx*rx*y+rx*rx;
				}

				if( y != ALastY )
				{
					ALastY = y;
					if( ACorners == 0xF )
					{
						drawFastHLine( xc-x,yc+y, 2 * x,color);

						if( y != 0 )
							drawFastHLine( xc-x,yc-y, 2 * x,color);
					}

					else
					{
						if( ACorners & 8 )
							drawFastHLine( xc-x,yc+y, x,color);

						if( ACorners & 4 )
							drawFastHLine( xc,yc+y, x,color);

						if( ACorners & 1 )
							drawFastHLine( xc-x,yc-y, x,color);

						if( ACorners & 2 )
							drawFastHLine( xc,yc-y, x, color);
					}
				}
			}
		}

		virtual void ClearScreen( uint16_t color ) = 0;
		virtual uint16_t GetPixelColor( int16_t x, int16_t y ) = 0;

		bool getTextWrap()
		{
			return wrap;
		}

		virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) 
		{
			drawFastHLine(x, y, w, color);
			drawFastHLine(x, y+h-1, w, color);
			drawFastVLine(x, y, h, color);
			drawFastVLine(x+w-1, y, h, color);
		}

		virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) 
		{
			// Update in subclasses if desired!
			for (int16_t i=x; i<x+w; i++)
				drawFastVLine(i, y, h, color);

		}

		virtual void fillScreen(uint16_t color) 
		{
			fillRect(0, 0, _width, _height, color);
		}

		// Draw a 1-bit color bitmap at the specified x, y position from the
		// provided bitmap buffer (must be PROGMEM memory) using color as the
		// foreground color and bg as the background color.
		void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg) 
		{
			int16_t i, j, byteWidth = (w + 7) / 8;
  
			for(j=0; j<h; j++) 
				for(i=0; i<w; i++ ) 
				{
					if(pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7)))
						drawPixel(x+i, y+j, color);

					else 
      					drawPixel(x+i, y+j, bg);
				}

		}


		void drawRoundRect( int16_t x, int16_t y, int16_t w, int16_t h, int16_t rx, int16_t ry, uint16_t color )
		{
			if( rx == 0 || ry == 0 )
			{
				drawRect( x, y, w, h, color );
				return;
			}

			drawFastHLine( x+rx , y    , w-2 * rx, color ); // Top
			drawFastHLine( x+rx , y+h-1, w-2 * rx, color ); // Bottom
			drawFastVLine( x    , y+ry , h-2 * ry, color ); // Left
			drawFastVLine( x+w-1, y+ry , h-2 * ry, color ); // Right

			drawEllipseHelper(x+rx,		y+ry,	  rx, ry, 1, color);
			drawEllipseHelper(x+w-rx-1, y+ry,	  rx, ry, 2, color);
			drawEllipseHelper(x+w-rx-1, y+h-ry-1, rx, ry, 4, color);
			drawEllipseHelper(x+rx,		y+h-ry-1, rx, ry, 8, color);
		}

		void fillRoundRect( int16_t x, int16_t y, int16_t w, int16_t h, int16_t rx, int16_t ry, uint16_t color )
		{
			if( rx == 0 || ry == 0 )
			{
				fillRect( x, y, w, h, color );
				return;
			}

			fillRect( x+rx, y, w-rx*2, ry+1, color );
			fillRect( x+rx, y+h-ry-1, w-rx*2, ry+1, color );
			fillRect( x, y+ry, w, h-ry * 2, color );
			fillEllipseHelper(x+rx,		y+ry,	  rx, ry, 1, color);
			fillEllipseHelper(x+w-rx-1, y+ry,	  rx, ry, 2, color);
			fillEllipseHelper(x+w-rx-1, y+h-ry-1, rx, ry, 4, color);
			fillEllipseHelper(x+rx,		y+h-ry-1, rx, ry, 8, color);
		}

		// Bresenham's algorithm - thx wikpedia
		virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) 
		{
			int16_t steep = abs(y1 - y0) > abs(x1 - x0);
			if (steep) 
			{
				swap(x0, y0);
				swap(x1, y1);
			}

			if (x0 > x1) 
			{
				swap(x0, x1);
				swap(y0, y1);
			}

			int16_t dx, dy;
			dx = x1 - x0;
			dy = abs(y1 - y0);

			int16_t err = dx / 2;
			int16_t ystep;

			if (y0 < y1) 
				ystep = 1;

			else
				ystep = -1;

			for (; x0<=x1; x0++) 
			{
				if (steep)
					drawPixel(y0, x0, color);
				else
					drawPixel(x0, y0, color);
			
				err -= dy;
				if (err < 0) 
				{
					y0 += ystep;
					err += dx;
				}
			}
		}

		virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) 
		{
		  // Update in subclasses if desired!
		  drawLine(x, y, x, y+h-1, color);
		}

		virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) 
		{
		  // Update in subclasses if desired!
		  drawLine(x, y, x+w-1, y, color);
		}

		virtual size_t write( uint8_t c )
		{
			if( cursor_y >= _height )
			{
				int ACorrection = ( cursor_y - _height ) + textsize * 8;
				Scroll( 0, -ACorrection, BacgroundColor );
				cursor_y -= ACorrection;
			}

			if (c == '\n') 
			{
				cursor_y += textsize*8;
				cursor_x  = 0;
			} 
			else if (c == '\r') 
			{
				cursor_x  = 0;
			} 
			else 
			{
				drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
				cursor_x += textsize*6;
				if (wrap && (cursor_x > (_width - textsize*6))) 
				{
					cursor_y += textsize*8;
					cursor_x = 0;
				}
			}

			return 1;
		}

		// Draw a character
		void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) 
		{
		  if((x >= _width)            || // Clip right
			 (y >= _height)           || // Clip bottom
			 ((x + 6 * size - 1) < 0) || // Clip left
			 ((y + 8 * size - 1) < 0))   // Clip top
			return;

		  if(!_cp437 && (c >= 176)) 
			  c++; // Handle 'classic' charset behavior

		  for (int8_t i=0; i<6; i++ ) 
		  {
			uint8_t line;
			if (i == 5) 
			  line = 0x0;

			else 
			  line = pgm_read_byte( FFont + 3 + (c*5)+i); // For now ifnore size information

			for (int8_t j = 0; j<8; j++) 
			{
			  if (line & 0x1) 
			  {
				if (size == 1) // default size
				  drawPixel(x+i, y+j, color);

				else  // big size
				  fillRect(x+(i*size), y+(j*size), size, size, color);
				 
			  }

			  else if (bg != color) 
			  {
				if (size == 1) // default size
				  drawPixel(x+i, y+j, bg);

				else  // big size
				  fillRect(x+i*size, y+j*size, size, size, bg);
				
			  }

			  line >>= 1;
			}
		  }
		}

		void setCursor(int16_t x, int16_t y) 
		{
			cursor_x = x;
			cursor_y = y;
		}

		int16_t getCursor( int16_t &x, int16_t &y ) const 
		{
			x = cursor_x;
			y = cursor_y;
		}

		void setTextSize(uint8_t s) 
		{
			textsize = (s > 0) ? s : 1;
		}

		void setTextColor(uint16_t c, uint16_t b) 
		{
			textcolor   = c;
			textbgcolor = b; 
		}

		void setTextWrap(boolean w) 
		{
			wrap = w;
		}

		uint8_t getRotation(void) const 
		{
			return rotation;
		}

		void setRotation(uint8_t x) 
		{
		  rotation = (x & 3);
		  switch(rotation) {
		   case 0:
		   case 2:
			_width  = WIDTH;
			_height = HEIGHT;
			break;
		   case 1:
		   case 3:
			_width  = HEIGHT;
			_height = WIDTH;
			break;
		  }
		}

		void cp437(boolean x) 
		{
			_cp437 = x;
		}

		// Return the size of the display (per current rotation)
		int16_t width(void) const 
		{
			return _width;
		}
 
		int16_t height(void) const 
		{
			return _height;
		}

	public:
		Graphics( int16_t w, int16_t h, const unsigned char * AFont ) :
			WIDTH(w), HEIGHT(h),
//			inherited( w, h ),
			FFont( AFont )
		{
			_width    = WIDTH;
			_height   = HEIGHT;
			rotation  = 0;
			cursor_y  = cursor_x    = 0;
			textsize  = 1;
			textcolor = textbgcolor = 0xFFFF;
			wrap      = true;
			_cp437    = false;
		}
	};
//---------------------------------------------------------------------------
	class GraphicsElementFillScreen : public GraphicsElementClocked
	{
		typedef GraphicsElementClocked inherited;

	public:
		TArduinoMonochromeColor	Color = tmcBlack;

	public:
		virtual void Render( bool AForce ) override
		{
			if( ! AForce )
				if( ClockInputPin.IsConnected() )
					return;

			FOwner.GetGraphics()->ClearScreen( Color );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class GraphicsElementDrawRectangle : public GraphicsElementClocked
	{
		typedef GraphicsElementClocked inherited;

	public:
		TArduinoMonochromeColor	Color = tmcWhite;
		TArduinoMonochromeColor	FillColor = tmcNone;
		int32_t		X = 0;
		int32_t		Y = 0;
		uint32_t	Width = 40;
		uint32_t	Height = 20;

	public:
		virtual void Render( bool AForce ) override
		{
			if( ! AForce )
				if( ClockInputPin.IsConnected() )
					return;

			int32_t AParentX, AParentY;
			FOwner.GetPosition( AParentX, AParentY );

			if( FillColor != tmcNone )
				FOwner.GetGraphics()->fillRect( AParentX + X, AParentY + Y, Width, Height, FillColor );

			if( Color != tmcNone )
				FOwner.GetGraphics()->drawRect( AParentX + X, AParentY + Y, Width, Height, Color );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class TArduinoGraphicsCorverSize
	{
	public:
		int16_t		X = 10;
		int16_t		Y = 10;
	};
//---------------------------------------------------------------------------
	class GraphicsElementDrawRoundRectangle : public GraphicsElementClocked
	{
		typedef GraphicsElementClocked inherited;

	public:
		TArduinoMonochromeColor	Color = tmcWhite;
		TArduinoMonochromeColor	FillColor = tmcNone;
		int32_t		X = 0;
		int32_t		Y = 0;
		uint32_t	Width = 40;
		uint32_t	Height = 20;
		TArduinoGraphicsCorverSize	CornerSize;

	public:
		virtual void Render( bool AForce ) override
		{
			if( ! AForce )
				if( ClockInputPin.IsConnected() )
					return;

			int32_t AParentX, AParentY;
			FOwner.GetPosition( AParentX, AParentY );

			if( FillColor != tmcNone )
				FOwner.GetGraphics()->fillRoundRect( AParentX + X, AParentY + Y, Width, Height, CornerSize.X, CornerSize.Y, FillColor );

			if( Color != tmcNone )
				FOwner.GetGraphics()->drawRoundRect( AParentX + X, AParentY + Y, Width, Height, CornerSize.X, CornerSize.Y, Color );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class GraphicsElementDrawEllipse : public GraphicsElementClocked
	{
		typedef GraphicsElementClocked inherited;

	public:
		TArduinoMonochromeColor	Color = tmcWhite;
		TArduinoMonochromeColor	FillColor = tmcNone;
		int32_t		X = 0;
		int32_t		Y = 0;
		uint32_t	Width = 40;
		uint32_t	Height = 20;

	public:
		virtual void Render( bool AForce ) override
		{
			if( ! AForce )
				if( ClockInputPin.IsConnected() )
					return;

			int32_t AParentX, AParentY;
			FOwner.GetPosition( AParentX, AParentY );

			if( FillColor != tmcNone )
				FOwner.GetGraphics()->fillEllipse( AParentX + X + Width / 2, AParentY + Y + Height / 2, ( Width + 1 ) / 2, ( Height + 1 ) / 2, FillColor );

			if( Color != tmcNone )
				FOwner.GetGraphics()->drawEllipse( AParentX + X + Width / 2, AParentY + Y + Height / 2, ( Width + 1 ) / 2, ( Height + 1 ) / 2, Color );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class GraphicsElementDrawPixel : public GraphicsElementClocked
	{
		typedef GraphicsElementClocked inherited;

	public:
		TArduinoMonochromeColor	Color = tmcWhite;
		int32_t		X = 0;
		int32_t		Y = 0;

	public:
		virtual void Render( bool AForce ) override
		{
			if( Color == tmcNone )
				return;

			if( ! AForce )
				if( ClockInputPin.IsConnected() )
					return;

			int32_t AParentX, AParentY;
			FOwner.GetPosition( AParentX, AParentY );

			FOwner.GetGraphics()->drawPixel( AParentX + X, AParentY + Y, Color );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class GraphicsElementDrawLine : public GraphicsElementClocked
	{
		typedef GraphicsElementClocked inherited;

	public:
		TArduinoMonochromeColor	Color = tmcWhite;
		int32_t		X = 0;
		int32_t		Y = 0;
		int32_t		Width = 40;
		int32_t		Height = 20;

	public:
		virtual void Render( bool AForce ) override
		{
			if( Color == tmcNone )
				return;

			if( ! AForce )
				if( ClockInputPin.IsConnected() )
					return;

			int32_t AParentX, AParentY;
			FOwner.GetPosition( AParentX, AParentY );

			FOwner.GetGraphics()->drawLine( AParentX + X, AParentY + Y, AParentX + X + Width, AParentY + Y + Height, Color );
//			FOwner.GetGraphics()->invalidate();
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class GraphicsElementDrawBitmap : public Mitov::GraphicsElementClocked
	{
		typedef Mitov::GraphicsElementClocked inherited;

    public:
		TArduinoMonochromeColor	Color = tmcWhite;
		TArduinoMonochromeColor	FillColor = tmcNone;
		int32_t	X = 0;
		int32_t	Y = 0;
		uint8_t	Width;
		uint8_t	Height;

		const unsigned char *_Bytes;

	public:
		virtual void Render( bool AForce ) override
		{
			if(( FillColor == tmcNone ) && ( Color == tmcNone ))
				return;

			if( ! AForce )
				if( ClockInputPin.IsConnected() )
					return;

			int32_t AParentX, AParentY;
			FOwner.GetPosition( AParentX, AParentY );

			FOwner.GetGraphics()->drawBitmap( AParentX + X, AParentY + Y, _Bytes, Width, Height, Color, FillColor );			
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class GraphicsElementDrawText : public GraphicsElementClocked, public GraphicsTextIntf
	{
		typedef GraphicsElementClocked inherited;

	public:
		TArduinoMonochromeColor	Color = tmcWhite;
		TArduinoMonochromeColor	FillColor = tmcNone;
		int32_t	X = 0;
		int32_t	Y = 0;

		String	Text;

		bool	Wrap = true;

	public:
		virtual void Render( bool AForce ) override
		{
			if(( FillColor == tmcNone ) && ( Color == tmcNone ))
				return;

			if( ! AForce )
				if( ClockInputPin.IsConnected() )
					return;

			int32_t AParentX, AParentY;
			FOwner.GetPosition( AParentX, AParentY );

			Graphics *AGraphics = FOwner.GetGraphics();
			int16_t ACursorY; // = AGraphics->getCursorY();
			int16_t ACursorX; // = AGraphics->getCursorX();

			AGraphics->getCursor( ACursorX, ACursorY );
			uint16_t AOldColor;
			uint16_t AOldBackgroundColor;

			bool AOldWrap = AGraphics->getTextWrap();

			for( int i = 0; i < FElements.size(); ++ i )
				FElements[ i ]->Enter( AGraphics );

			AGraphics->setTextWrap( Wrap );			

			AGraphics->getTextColor( AOldColor, AOldBackgroundColor );

			AGraphics->setTextColor( Color, FillColor );

			AGraphics->setCursor( X, Y );
			AGraphics->print( Text );

			AGraphics->setTextColor( AOldColor, AOldBackgroundColor );

			AGraphics->setCursor( ACursorX, ACursorY );

			AGraphics->setTextWrap( AOldWrap );

			for( int i = 0; i < FElements.size(); ++ i )
				FElements[ i ]->Leave( AGraphics );

		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class GraphicsElementTextFeld : public GraphicsElementBasic, public GraphicsTextIntf
	{
		typedef GraphicsElementBasic inherited;

	public:
		OpenWire::ValueSinkPin<String>	InputPin;

	public:
		int32_t	X = 0;
		int32_t	Y = 0;
		uint16_t Width = 6;
		TArduinoMonochromeColor	Color : 2;
		TArduinoMonochromeColor	FillColor : 2;
		TArduinoTextHorizontalAlign	HorizontalAlign : 2;
		bool AutoSize : 1;
		bool Wrap : 1;

	public:
		virtual void Render( bool AForce ) override
		{
			if(( FillColor == tmcNone ) && ( Color == tmcNone ))
				return;

			int32_t AParentX, AParentY;
			FOwner.GetPosition( AParentX, AParentY );

			Graphics *AGraphics = FOwner.GetGraphics();
			int16_t ACursorY; // = AGraphics->getCursorY();
			int16_t ACursorX; // = AGraphics->getCursorX();

			AGraphics->getCursor( ACursorX, ACursorY );

			uint16_t AOldColor;
			uint16_t AOldBackgroundColor;

			for( int i = 0; i < FElements.size(); ++ i )
				FElements[ i ]->Enter( AGraphics );

			bool AOldWrap = AGraphics->getTextWrap();

			AGraphics->setTextWrap( Wrap );			

			AGraphics->getTextColor( AOldColor, AOldBackgroundColor );

			AGraphics->setTextColor( Color, FillColor );

			AGraphics->setCursor( X, Y );
			if( AutoSize )
				AGraphics->print( InputPin.Value );

			else
			{
				String AText = InputPin.Value;
				while( AText.length() < Width )
				{
					switch( HorizontalAlign )
					{
						case thaLeft :
							AText += " ";
							break;

						case thaRight :
							AText = " " + AText;
							break;

						case thaCenter :
							AText = " " + AText + " ";
							break;
					}
				}

				AText = AText.substring( 0, Width );
				AGraphics->print( AText );
			}

			AGraphics->setTextColor( AOldColor, AOldBackgroundColor );
			AGraphics->setCursor( ACursorX, ACursorY );
			AGraphics->setTextWrap( AOldWrap );			

			for( int i = 0; i < FElements.size(); ++ i )
				FElements[ i ]->Leave( AGraphics );

		}

	protected:
		void DoReceiveData( void *_Data )
		{
			Render( true );
		}

	public:
		GraphicsElementTextFeld( GraphicsIntf &AOwner ) :
			inherited( AOwner ),
			HorizontalAlign( thaLeft ),
			AutoSize( true ),
			Wrap( true ),
			Color( tmcWhite ),
			FillColor( tmcBlack )
		{
			InputPin.SetCallback( this, (OpenWire::TOnPinReceive)&GraphicsElementTextFeld::DoReceiveData );
		}

	};
//---------------------------------------------------------------------------
	struct GraphicsPoint
	{
		int32_t	X;
		int32_t	Y;
	};
//---------------------------------------------------------------------------
	class GraphicsElementDrawLines : public GraphicsElementClocked
	{
		typedef GraphicsElementClocked inherited;

	public:
		TArduinoMonochromeColor	Color = tmcWhite;
		int32_t	X = 0;
		int32_t	Y = 0;

	public:
		GraphicsPoint	*Points;
		uint32_t	_PointsCount = 0;

	public:
		virtual void Render( bool AForce ) override
		{
			if( _PointsCount == 0 )
				return;

			if( Color == tmcNone )
				return;

			if( ! AForce )
				if( ClockInputPin.IsConnected() )
					return;

			int32_t AParentX, AParentY;
			FOwner.GetPosition( AParentX, AParentY );

			Graphics *AGraphics = FOwner.GetGraphics();

			int32_t	AX1 = X;
			int32_t	AY1 = Y;

			for( int i = 0; i < _PointsCount; ++i )
			{
				int32_t	AX2 = Points[ i ].X + X;
				int32_t	AY2 = Points[ i ].Y + Y;

				AGraphics->drawLine( AParentX + AX1, AParentY + AY1, AParentX + AX2, AParentY + AY2, Color );

				AX1 = AX2;
				AY1 = AY2;
			}
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class GraphicsElementDrawPolygon : public GraphicsElementClocked
	{
		typedef GraphicsElementClocked inherited;

	public:
		TArduinoMonochromeColor	Color = tmcWhite;
		TArduinoMonochromeColor	FillColor = tmcNone;
		int32_t	X = 0;
		int32_t	Y = 0;

	public:
		GraphicsPoint	*Points;
		uint32_t	_PointsCount = 0;

	public:
		virtual void Render( bool AForce ) override
		{
			if( _PointsCount == 0 )
				return;

			if(( FillColor == tmcNone ) && ( Color == tmcNone ))
				return;

			if( ! AForce )
				if( ClockInputPin.IsConnected() )
					return;

			int32_t AParentX, AParentY;
			FOwner.GetPosition( AParentX, AParentY );

			Graphics *AGraphics = FOwner.GetGraphics();

			int32_t	AStartX = AParentX + X;
			int32_t	AStartY = AParentY + Y;

			if( FillColor != tmcNone )
			{
				int32_t	AMinX = AStartX;
				int32_t	AMinY = AStartY;
				int32_t	AMaxX = AStartX;
				int32_t	AMaxY = AStartY;

				for( int i = 0; i < _PointsCount; ++i )
				{
					int32_t	AX2 = Points[ i ].X + AStartX;
					int32_t	AY2 = Points[ i ].Y + AStartY;

					if( AX2 < AMinX )
						AMinX = AX2;

					if( AX2 > AMaxX )
						AMaxX = AX2;

					if( AY2 < AMinY )
						AMinY = AY2;

					if( AY2 > AMaxY )
						AMaxY = AY2;

				}

				if( AMinX < 0 )
				 AMinX = 0;

				if( AMinY < 0 )
				 AMinY = 0;

				int32_t	AWidth = AGraphics->width();
				int32_t	AHeight = AGraphics->height();

				if( AMaxX > AWidth )
				 AMaxX = AWidth;

				if( AMaxY > AHeight )
				 AMaxY = AHeight;

				if( AMinX > AMaxX )
					return;

				if( AMinY > AMaxY )
					return;

				int32_t *nodeX = new int32_t[ _PointsCount ];
				for( int pixelY = AMinY; pixelY < AMaxY; ++ pixelY )
				{
					//  Build a list of nodes.
					int nodes = 0;
					int j = _PointsCount - 1;
					for( int i = 0; i < _PointsCount; i++ )
					{
						if ( ( Points[ i ].Y + AStartY ) < pixelY && ( Points[ j ].Y + AStartY ) >= pixelY || ( Points[ j ].Y + AStartY ) < pixelY && ( Points[ i ].Y + AStartY ) >= pixelY )
							nodeX[nodes ++] = ( Points[i].X + AStartX ) + float( pixelY - ( Points[i].Y + AStartY ) ) / ( Points[j].Y - Points[i].Y ) * float( (Points[j].X - Points[i].X ) ) + 0.5;

						j = i;
					}

					//  Sort the nodes, via a simple “Bubble” sort.
					int i = 0;
					while( i < nodes - 1 )
					{
						if( nodeX[ i ] > nodeX[ i + 1 ] )
						{
							int32_t swap = nodeX[ i ];
							nodeX[ i ] = nodeX[ i + 1 ];
							nodeX[ i + 1 ] = swap;
							if( i )
								i--;
						}

						else
							i++;
						
					}

					//  Fill the pixels between node pairs.
					for( int i = 0; i < nodes; i += 2 ) 
					{
						int32_t ALeft = nodeX[ i ];
						int32_t ARight = nodeX[ i + 1 ];

						if( ALeft >= AWidth )
							break;

						if( ARight > 0 ) 
						{
							if( ALeft < 0 ) 
								ALeft = 0;

							if( ARight > AWidth )
								ARight = AWidth;

							AGraphics->drawFastHLine( ALeft, pixelY, ARight - ALeft, FillColor );

	//					for (pixelX=nodeX[i]; pixelX<nodeX[i+1]; pixelX++) fillPixel(pixelX,pixelY); 
						}
					}

				}

			}

			if( Color != tmcNone )
			{
				int32_t	AX1 = X;
				int32_t	AY1 = Y;

				for( int i = 1; i < _PointsCount; ++i )
				{
					int32_t	AX2 = Points[ i ].X + X;
					int32_t	AY2 = Points[ i ].Y + Y;

					AGraphics->drawLine( AParentX + AX1, AParentY + AY1, AParentX + AX2, AParentY + AY2, Color );

					AX1 = AX2;
					AY1 = AY2;
				}

				AGraphics->drawLine( AParentX + AX1, AParentY + AY1, AParentX + X, AParentY + Y, Color );
			}
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class GraphicsElementScroll : public GraphicsElementClocked
	{
		typedef GraphicsElementClocked inherited;

	public:
		TArduinoMonochromeColor	Color = tmcBlack;
		int32_t	X = 0;
		int32_t	Y = 0;

	public:
		virtual void Render( bool AForce ) override
		{
			if( ! AForce )
				if( ClockInputPin.IsConnected() )
					return;

			FOwner.GetGraphics()->Scroll( X, Y, Color );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class GraphicsElementSetCursor : public GraphicsElementClocked
	{
		typedef GraphicsElementClocked inherited;

	public:
		uint32_t X = 0;
		uint32_t Y = 0;

	public:
		virtual void Render( bool AForce ) override
		{
			if( ! AForce )
				if( ClockInputPin.IsConnected() )
					return;

			FOwner.GetGraphics()->setCursor( X, Y );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class GraphicsElementCheckPixel : public GraphicsElementClocked
	{
		typedef GraphicsElementClocked inherited;

	public:
		OpenWire::SourcePin	OutputPin;

	public:
		uint32_t X = 0;
		uint32_t Y = 0;

	public:
		virtual void Render( bool AForce ) override
		{
			if( ! AForce )
				if( ClockInputPin.IsConnected() )
					return;

			uint16_t AValue = FOwner.GetGraphics()->GetPixelColor( X, Y );
			OutputPin.SendValue<bool>( AValue );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class GraphicsElementDrawScene : public GraphicsElementClocked, public GraphicsIntf
	{
		typedef GraphicsElementClocked inherited;

	public:
		int32_t X = 0;
		int32_t Y = 0;

	public:
		virtual void GetPosition( int32_t &AX, int32_t &AY ) override
		{ 
			int32_t AParentX, AParentY;
			FOwner.GetPosition( AParentX, AParentY );

			AX = AParentX + X;
			AY = AParentY + Y;
		}

		virtual Graphics *GetGraphics() override
		{
			return FOwner.GetGraphics();
		}

	public:
		virtual void Render( bool AForce ) override
		{
			for( int i = 0; i < FElements.size(); ++ i )
				FElements[ i ]->Render( AForce );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class GraphicsTextElementFont : public GraphicsTextElementBasic
	{
		typedef GraphicsTextElementBasic inherited;

	protected:
		const unsigned char *FFont;

	protected:
		const unsigned char *FOldFont;

	public:
		virtual void Enter( Graphics *AGraphics )
		{
			FOldFont = AGraphics->getFont();
			AGraphics->setFont( FFont );
		}

		virtual void Leave( Graphics *AGraphics ) 
		{
			AGraphics->setFont( FOldFont );
		}

	public:
		GraphicsTextElementFont( GraphicsTextIntf &AOwner, const unsigned char *AFont ) :
			inherited( AOwner ),
			FFont( AFont )
		{
		}

	};
//---------------------------------------------------------------------------
}

#endif
