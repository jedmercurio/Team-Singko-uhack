////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_DISPLAY_SSD1306_h
#define _MITOV_DISPLAY_SSD1306_h

#include <Mitov.h>
#include <Mitov_Graphics.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	class TArduinoDisplaySSD1306PreChargePeriod
	{
	public:
		uint8_t	Phase1 : 4;
		uint8_t	Phase2 : 4;

	public:
		TArduinoDisplaySSD1306PreChargePeriod() :
			Phase1( 1 ),
			Phase2( 15 )
		{
		}
	};
//---------------------------------------------------------------------------
	class TArduinoDisplaySSD1306TextSettings
	{
	public:
		uint8_t	Size : 4;
		bool	Wrap : 1;
		TArduinoMonochromeColor	Color : 2;
		TArduinoMonochromeColor	BackgroundColor : 2;

	public:
		TArduinoDisplaySSD1306TextSettings() :
			Size( 1 ),
			Wrap( true ),
			Color( tmcWhite ),
			BackgroundColor( tmcNone )
		{
		}
	};
//---------------------------------------------------------------------------
	class DisplaySSD1306I2C : public OpenWire::Component, public Graphics
	{
		typedef OpenWire::Component inherited;

	protected:
		static const uint8_t SSD1306_SETCONTRAST = 0x81;
		static const uint8_t SSD1306_DISPLAYALLON_RESUME = 0xA4;
		static const uint8_t SSD1306_DISPLAYALLON = 0xA5;
		static const uint8_t SSD1306_NORMALDISPLAY = 0xA6;
		static const uint8_t SSD1306_INVERTDISPLAY = 0xA7;
		static const uint8_t SSD1306_DISPLAYOFF = 0xAE;
		static const uint8_t SSD1306_DISPLAYON = 0xAF;

		static const uint8_t SSD1306_SETDISPLAYOFFSET = 0xD3;
		static const uint8_t SSD1306_SETCOMPINS = 0xDA;

		static const uint8_t SSD1306_SETVCOMDETECT = 0xDB;

		static const uint8_t SSD1306_SETDISPLAYCLOCKDIV = 0xD5;
		static const uint8_t SSD1306_SETPRECHARGE = 0xD9;

		static const uint8_t SSD1306_SETMULTIPLEX = 0xA8;

		static const uint8_t SSD1306_SETLOWCOLUMN = 0x00;
		static const uint8_t SSD1306_SETHIGHCOLUMN = 0x10;

		static const uint8_t SSD1306_SETSTARTLINE = 0x40;

		static const uint8_t SSD1306_MEMORYMODE = 0x20;
		static const uint8_t SSD1306_COLUMNADDR = 0x21;
		static const uint8_t SSD1306_PAGEADDR = 0x22;

		static const uint8_t SSD1306_COMSCANINC = 0xC0;
		static const uint8_t SSD1306_COMSCANDEC = 0xC8;

		static const uint8_t SSD1306_SEGREMAP = 0xA0;

		static const uint8_t SSD1306_CHARGEPUMP = 0x8D;

		static const uint8_t SSD1306_EXTERNALVCC = 0x1;
		static const uint8_t SSD1306_SWITCHCAPVCC = 0x2;

	public:
		OpenWire::SourcePin	ResetOutputPin;
		OpenWire::ConnectSinkPin	RefreshInputPin;

	public:
		uint8_t	Address = 0x3C;
		bool	UseChargePump = false;
		float	Contrast = 0.812;

		TArduinoDisplaySSD1306PreChargePeriod	PreChargePeriod;

		TArduinoDisplaySSD1306TextSettings	Text;

	protected:
		uint8_t *buffer;
		bool	FModified = false;

	public:
		void UpdateContrast()
		{
			SendCommand( SSD1306_SETCONTRAST );
			SendCommand( Contrast * 255 + 0.5 );
		}

//	protected:
//		Adafruit_SSD1306	FDisplay = Adafruit_SSD1306( 0 );

	public:
		template<typename T> void Print( T AValue )
		{
/*
			if( cursor_y >= _height )
			{
				int ACorrection = ( cursor_y - _height ) + textsize * 8;
//				ScrollBufferUp( ACorrection );
				Scroll( 0, -ACorrection, BacgroundColor );
				cursor_y -= ACorrection;
			}
*/
			println( AValue );
//			display();
//			Serial.println( AValue );
//			FDisplay.println( AValue );
		}

	public:
		virtual void drawPixel(int16_t x, int16_t y, uint16_t color) override
		{
			if( color == tmcNone )
				return;

			if ((x < 0) || (x >= _width ) || (y < 0) || (y >= _height ))
				return;

			// check rotation, move pixel around if necessary
			switch (getRotation()) 
			{
				case 1:
					swap(x, y);
					x = _width - x - 1;
					break;

				case 2:
					x = _width - x - 1;
					y = _height - y - 1;
					break;

				case 3:
					swap(x, y);
					y = _height - y - 1;
					break;
				}  

			// x is which column
			switch (color) 
			{
				case tmcWhite :		buffer[x+ (y/8) * _width ] |=  (1 << (y&7)); break;
				case tmcBlack :		buffer[x+ (y/8) * _width ] &= ~(1 << (y&7)); break; 
				case tmcInvert :	buffer[x+ (y/8) * _width ] ^=  (1 << (y&7)); break; 
			}

			FModified = true;
    	}

		virtual void ClearScreen( uint16_t color ) override
		{
			switch( color )
			{
			case tmcBlack :
				memset(buffer, 0, ( _width * _height / 8));
				break;

			case tmcWhite :
				memset(buffer, 255, ( _width * _height / 8));
				break;

			case tmcInvert :
				for( int i = 0; i < _width * _height / 8; ++i )
					buffer[ i ] = ~buffer[ i ];

				break;

			}
		}

		uint16_t GetPixelColor( int16_t x, int16_t y )
		{
			return ( buffer[ x + ( y / 8 ) * _width ] & ( y & 7 ) != 0 );
		}

		void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) 
		{
			bool bSwap = false;
			switch(rotation) 
			{ 
//				case 0:
					// 0 degree rotation, do nothing
//					break;
				case 1:
					// 90 degree rotation, swap x & y for rotation, then invert x
					bSwap = true;
					swap(x, y);
					x = _width - x - 1;
					break;
				case 2:
					// 180 degree rotation, invert x and y - then shift y around for height.
					x = _width - x - 1;
					y = _height - y - 1;
					x -= (w-1);
					break;
				case 3:
					// 270 degree rotation, swap x & y for rotation, then invert y  and adjust y for w (not to become h)
					bSwap = true;
					swap(x, y);
					y = _height - y - 1;
					y -= (w-1);
					break;
			}

			if( bSwap ) 
				drawFastVLineInternal(x, y, w, color);

			else
				drawFastHLineInternal(x, y, w, color);
		}

		virtual void Scroll( int32_t X, int32_t Y, uint16_t color ) override
		{
			if( X == 0 && Y == 0 )
				return;

			uint8_t	AOffset = abs( Y ) % 8;
			int32_t	AYBytes = Y / 8;
			AYBytes *= _width;

/*
			if( ! Y )
			{
				uint8_t *ABuffer = buffer;
				if( X > 0 )
					for( int ayi = 0; ayi < _height / 8; ++ ayi, ABuffer += _width )
					{
						memmove( ABuffer + X, ABuffer, _width - X );
						switch( color )
						{
							case tmcBlack:
								memset( ABuffer, 0, X ); break;

							case tmcWhite:
								memset( ABuffer, 255, X ); break;

							case tmcInvert:
								for( int axi = 0; axi < X; ++ axi )
									ABuffer[ axi ] = ~ABuffer[ axi ];
						}
					}

				else // Y = 0 && X < 0
					for( int ayi = 0; ayi < _height / 8; ++ ayi, ABuffer += _width )
					{
						memmove( ABuffer, ABuffer - X, _width + X );
						switch( color )
						{
							case tmcBlack:
								memset( ABuffer + _width + X, 0, -X ); break;

							case tmcWhite:
								memset( ABuffer + _width + X, 255, -X ); break;

							case tmcInvert:
								for( int axi = _width + X; axi < _width; ++ axi )
									ABuffer[ axi ] = ~ABuffer[ axi ];
						}
					}

			}

			else // Y <> 0
*/
			{
				if( Y > 0 )
				{
					if( X == 0 )
					{
						if( AYBytes )
							memmove( buffer + AYBytes, buffer, ( _height / 8 ) * _width - AYBytes );

					}

					else if( X > 0 )
					{
						uint8_t *ABuffer = buffer + _width * (( ( _height - Y + 7 ) / 8 ) - 1 );
						for( int ayi = 0; ayi < ( _height - Y + 7 ) / 8; ++ ayi, ABuffer -= _width )
						{
							memmove( ABuffer + AYBytes + X, ABuffer, _width - X );
							switch( color )
							{
								case tmcBlack:
									memset( ABuffer + AYBytes, 0, X ); break;

								case tmcWhite:
									memset( ABuffer + AYBytes, 255, X ); break;

								case tmcInvert:
									for( int axi = 0; axi < X; ++ axi )
										ABuffer[ axi + AYBytes ] = ~ABuffer[ axi + AYBytes ];
							}
						}
					}

					else // Y > 0 && X < 0
					{ 
						uint8_t *ABuffer = buffer + _width * (( ( _height - Y + 7 ) / 8 ) - 1 );
						for( int ayi = 0; ayi < ( _height - Y + 7 ) / 8; ++ ayi, ABuffer -= _width )
						{
							memmove( ABuffer + AYBytes, ABuffer - X, _width + X );
							switch( color )
							{
								case tmcBlack:
									memset( ABuffer + AYBytes + _width + X, 0, -X ); break;

								case tmcWhite:
									memset( ABuffer + AYBytes + _width + X, 255, -X ); break;

								case tmcInvert:
									for( int axi = _width + X; axi < _width; ++ axi )
										ABuffer[ axi + AYBytes ] = ~ABuffer[ axi + AYBytes ];
							}
						}
					}

					uint8_t * AToPtr = buffer + _width * _height / 8 - 1;
					uint8_t *AFromPtr = AToPtr - _width;
					for( int ayi = 0; ayi < ( _height - Y + 7 ) / 8 - 1; ++ ayi )
						for( int axi = 0; axi < _width; ++ axi, --AFromPtr, --AToPtr )
							*AToPtr = *AToPtr << AOffset | ( *AFromPtr >> ( 8 - AOffset ));

					if( color != tmcNone )
					{
						uint8_t	AMask;
						if( color == tmcBlack )
							AMask = 0xFF << AOffset;

						else
							AMask = 0xFF >> ( 8 - AOffset );

						AToPtr = buffer + AYBytes;
						for( int axi = 0; axi < _width; ++ axi, ++AToPtr )
						{
							switch( color )
							{
								case tmcBlack:
									*AToPtr <<= AOffset;
									*AToPtr &= AMask; 
									break;

								case tmcWhite:
									*AToPtr <<= AOffset;
									*AToPtr |= AMask; 
									break;

								case tmcInvert:
									*AToPtr <<= AOffset | ( *AToPtr & ( ~AMask ));
									*AToPtr ^= AMask; 
									break;
							}
						}
					}
				}

				else // Y < 0
				{
					if( X == 0 )
					{
						if( AYBytes )
							memmove( buffer, buffer - AYBytes, ( _height / 8 ) * _width + AYBytes );

					}

					else if( X > 0 )
					{
						uint8_t *ABuffer = buffer;
						for( int ayi = 0; ayi < ( _height + Y + 7 ) / 8; ++ ayi, ABuffer += _width )
						{
							memmove( ABuffer + X, ABuffer - AYBytes, _width - X );
							switch( color )
							{
								case tmcBlack:
									memset( ABuffer, 0, X ); break;

								case tmcWhite:
									memset( ABuffer, 255, X ); break;

								case tmcInvert:
									for( int axi = 0; axi < X; ++ axi )
										ABuffer[ axi ] = ~ABuffer[ axi ];
							}
						}
					}
					else // Y < 0 && X < 0
					{
						uint8_t *ABuffer = buffer;
						for( int ayi = 0; ayi < ( _height + Y + 7 ) / 8; ++ ayi, ABuffer += _width )
						{
							memmove( ABuffer, ABuffer - AYBytes - X, _width + X );
							switch( color )
							{
								case tmcBlack:
									memset( ABuffer + _width + X, 0, -X ); break;

								case tmcWhite:
									memset( ABuffer + _width + X, 255, -X ); break;

								case tmcInvert:
									for( int axi = _width + X; axi < _width; ++ axi )
										ABuffer[ axi ] = ~ABuffer[ axi ];
							}
						}
					}

					if( AOffset )
					{
						uint8_t * AToPtr = buffer;
						uint8_t *AFromPtr = AToPtr + _width;
						for( int ayi = 0; ayi < ( _height + Y + 7 ) / 8 - 1; ++ ayi )
							for( int axi = 0; axi < _width; ++ axi, ++AFromPtr, ++AToPtr )
								*AToPtr = *AToPtr >> AOffset | ( *AFromPtr << ( 8 - AOffset ));

						if( color != tmcNone )
						{
							uint8_t	AMask;
							if( color == tmcBlack )
								AMask = 0xFF >> AOffset;

							else
								AMask = 0xFF << ( 8 - AOffset );

							AToPtr = buffer + _width * _height / 8 + AYBytes - _width;
							for( int axi = 0; axi < _width; ++ axi, ++AToPtr )
							{
								switch( color )
								{
									case tmcBlack:
										*AToPtr >>= AOffset;
										*AToPtr &= AMask; 
										break;

									case tmcWhite:
										*AToPtr >>= AOffset;
										*AToPtr |= AMask; 
										break;

									case tmcInvert:
										*AToPtr >>= AOffset | ( *AToPtr & ( ~AMask ));
										*AToPtr ^= AMask; 
										break;
								}
							}
						}
					}
				}

				if( AYBytes )
				{
					if( Y > 0 )
					{
						switch( color )
						{
							case tmcBlack:
								memset( buffer, 0, AYBytes ); break;

							case tmcWhite:
								memset( buffer, 255, AYBytes ); break;

							case tmcInvert:
								for( int axi = 0; axi < _width; ++ axi )
									buffer[ axi ] = ~buffer[ axi ];
						}
					}

					else // Y < 0
					{
						switch( color )
						{
							case tmcBlack:
								memset( buffer + _height * _width / 8 + AYBytes, 0, - AYBytes ); break;

							case tmcWhite:
								memset( buffer + _height * _width / 8 + AYBytes, 255, - AYBytes ); break;

							case tmcInvert:
								for( int axi = 0; axi < _width; ++ axi )
									buffer[ axi + _height * _width / 8 + AYBytes ] = ~buffer[ axi + _height * _width / 8 + AYBytes ];
						}
					}
				}
			}
		}

	protected:
/*
		void ScrollBufferUp( uint8_t ANumLines )
		{
			memmove( buffer, buffer + ANumLines * _width / 8, ( _height - ANumLines ) * _width / 8 );
			memset( buffer + ( _height - ANumLines ) * _width / 8, 0, ANumLines * _width / 8 );
		}
*/
		void drawFastHLineInternal(int16_t x, int16_t y, int16_t w, uint16_t color) 
		{
			if( color == tmcNone )
				return;

			// Do bounds/limit checks
			if(y < 0 || y >= _height) 
				return;

			// make sure we don't try to draw below 0
			if(x < 0) 
			{ 
				w += x;
				x = 0;
			}

			// make sure we don't go off the edge of the display
			if( (x + w) > _width) 
				w = (_width - x);

			// if our width is now negative, punt
			if(w <= 0) 
				return;

			// set up the pointer for  movement through the buffer
			register uint8_t *pBuf = buffer;
			// adjust the buffer pointer for the current row
			pBuf += ((y/8) * _width);
			// and offset x columns in
			pBuf += x;

			register uint8_t mask = 1 << (y&7);

			switch (color) 
			{
				case tmcWhite:         while(w--) { *pBuf++ |= mask; }; break;
				case tmcBlack: mask = ~mask;   while(w--) { *pBuf++ &= mask; }; break;
				case tmcInvert:         while(w--) { *pBuf++ ^= mask; }; break;
			}
		}

		void drawFastVLineInternal(int16_t x, int16_t __y, int16_t __h, uint16_t color) 
		{
			if( color == tmcNone )
				return;

			// do nothing if we're off the left or right side of the screen
			if(x < 0 || x >= _width) 
				return;

			// make sure we don't try to draw below 0
			if(__y < 0) 
			{ 
				// __y is negative, this will subtract enough from __h to account for __y being 0
				__h += __y;
				__y = 0;
			} 

			// make sure we don't go past the height of the display
			if( (__y + __h) > _height)
				__h = (_height - __y);
			
			// if our height is now negative, punt 
			if(__h <= 0)
				return;			

			// this display doesn't need ints for coordinates, use local byte registers for faster juggling
			register uint8_t y = __y;
			register uint8_t h = __h;


			// set up the pointer for fast movement through the buffer
			register uint8_t *pBuf = buffer;
			// adjust the buffer pointer for the current row
			pBuf += ((y/8) * _width);
			// and offset x columns in
			pBuf += x;

			// do the first partial byte, if necessary - this requires some masking
			register uint8_t mod = (y&7);
			if(mod) 
			{
				// mask off the high n bits we want to set 
				mod = 8-mod;

				// note - lookup table results in a nearly 10% performance improvement in fill* functions
				// register uint8_t mask = ~(0xFF >> (mod));
				static uint8_t premask[8] = {0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };
				register uint8_t mask = premask[mod];

				// adjust the mask if we're not going to reach the end of this byte
				if( h < mod)
					mask &= (0XFF >> (mod-h));

				switch (color) 
				{
					case tmcWhite :  *pBuf |=  mask;  break;
					case tmcBlack :  *pBuf &= ~mask;  break;
					case tmcInvert : *pBuf ^=  mask;  break;
				}
  
				// fast exit if we're done here!
				if(h<mod) { return; }

				h -= mod;

				pBuf += _width;
			}


			// write solid bytes while we can - effectively doing 8 rows at a time
			if(h >= 8) 
			{ 
				if (color == tmcInvert)  
				{          // separate copy of the code so we don't impact performance of the black/white write version with an extra comparison per loop
					do  
					{
						*pBuf=~(*pBuf);

						// adjust the buffer forward 8 rows worth of data
						pBuf += _width;

						// adjust h & y (there's got to be a faster way for me to do this, but this should still help a fair bit for now)
						h -= 8;
					} while(h >= 8);
				}

				else 
				{
					// store a local value to work with 
					register uint8_t val = (color == tmcWhite) ? 255 : 0;

					do  
					{
						// write our value in
						*pBuf = val;

						// adjust the buffer forward 8 rows worth of data
						pBuf += _width;

						// adjust h & y (there's got to be a faster way for me to do this, but this should still help a fair bit for now)
						h -= 8;
					} while(h >= 8);
				}
			}

			// now do the final partial byte, if necessary
			if(h) 
			{
				mod = h & 7;
				// this time we want to mask the low bits of the byte, vs the high bits we did above
				// register uint8_t mask = (1 << mod) - 1;
				// note - lookup table results in a nearly 10% performance improvement in fill* functions
				static uint8_t postmask[8] = {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F };
				register uint8_t mask = postmask[mod];
				switch (color) 
				{
					case tmcWhite:   *pBuf |=  mask;  break;
					case tmcBlack:   *pBuf &= ~mask;  break;
					case tmcInvert:  *pBuf ^=  mask;  break;
				}
			}
		}

		void SendCommand(uint8_t ACommand )
		{
//			Serial.println( ACommand, HEX );
			Wire.beginTransmission( Address );
			Wire.write( 0x00 ); // Co = 0, D/C = 0
			Wire.write( ACommand );
			Wire.endTransmission();
		}

		void SendCommands( uint8_t ACommands[], int ASize ) 
		{
			for( int i = 0; i < ASize; ++ i )
				SendCommand( ACommands[ i ] );
		}

	public:
		void display()
		{
			SendCommand(SSD1306_COLUMNADDR);
			SendCommand(0);   // Column start address (0 = reset)
			SendCommand( _width - 1 ); // Column end address (127 = reset)

			SendCommand(SSD1306_PAGEADDR);
			SendCommand(0); // Page start address (0 = reset)
			if( _height == 64 )
				SendCommand(7); // Page end address

			if( _height == 32 )
				SendCommand(3); // Page end address

			if( _height == 16 )
				SendCommand(1); // Page end address
			
			// save I2C bitrate
		#ifndef __SAM3X8E__
			uint8_t twbrbackup = TWBR;
			TWBR = 12; // upgrade to 400KHz!
		#endif

			//Serial.println(TWBR, DEC);
			//Serial.println(TWSR & 0x3, DEC);

			// I2C
			for( uint16_t i=0; i < ( _width * _height /8); ) 
			{
				// send a bunch of data in one xmission
				Wire.beginTransmission( Address );
				Wire.write( 0x40 );
				Wire.write( buffer + i, 16 );
				i += 16;
				Wire.endTransmission();
			}

		#ifndef __SAM3X8E__
			TWBR = twbrbackup;
		#endif
			FModified = false;
		}

	protected:
		virtual void SystemLoopEnd() override
		{
			if( FModified )
				if( ! RefreshInputPin.IsConnected() )
					display();

		}

		virtual void SystemInit() override
		{
//			Serial.println( "Test1" );
#ifdef __SAM3X8E__
			// Force 400 KHz I2C, rawr! (Uses pins 20, 21 for SDA, SCL)
			TWI1->TWI_CWGR = 0;
			TWI1->TWI_CWGR = ((VARIANT_MCK / (2 * 400000)) - 4) * 0x101;
#endif
			if( ResetOutputPin.IsConnected() )
			{
				ResetOutputPin.SendValue( true );
				// VDD (3.3V) goes high at start, lets just chill for a ms
				delay(1);
				// bring reset low
				ResetOutputPin.SendValue( false );
				// wait 10ms
				delay(10);
				// bring out of reset
				ResetOutputPin.SendValue( true );
				// turn on VCC (9V?)
			}

			uint8_t InitCommands1[] =
			{
				SSD1306_DISPLAYOFF,                    // 0xAE
				SSD1306_SETDISPLAYCLOCKDIV,            // 0xD5
				0x80,                                  // the suggested ratio 0x80
				SSD1306_SETMULTIPLEX,                  // 0xA8
				0x3F,
				SSD1306_SETDISPLAYOFFSET,              // 0xD3
				0x0,                                   // no offset
				SSD1306_SETSTARTLINE | 0x0,            // line #0
				SSD1306_CHARGEPUMP                     // 0x8D
			};

			// Init sequence for 128x64 OLED module
			SendCommands( ARRAY_PARAM( InitCommands1 ) );
			SendCommand( UseChargePump ? 0x10 : 0x14 );

			uint8_t InitCommands2[] =
			{
				SSD1306_MEMORYMODE,                    // 0x20
				0x00,                                  // 0x0 act like ks0108
				SSD1306_SEGREMAP | 0x1,
				SSD1306_COMSCANDEC,
				SSD1306_SETCOMPINS,                    // 0xDA
				0x12
//					SSD1306_SETCONTRAST                    // 0x81
			};

			SendCommands( ARRAY_PARAM( InitCommands2 ) );
			UpdateContrast();

			SendCommand(SSD1306_SETPRECHARGE);                  // 0xd9
			SendCommand( ( uint8_t( PreChargePeriod.Phase2 ) << 4 ) | PreChargePeriod.Phase1 );

			uint8_t InitCommands3[] =
			{
				SSD1306_SETVCOMDETECT,                 // 0xDB
				0x40,
				SSD1306_DISPLAYALLON_RESUME,           // 0xA4
				SSD1306_NORMALDISPLAY,                 // 0xA6
				SSD1306_DISPLAYON
			};

			SendCommands( ARRAY_PARAM( InitCommands3 ) );

			setTextSize( Text.Size );
			setTextColor( Text.Color, Text.BackgroundColor );
			setTextWrap( Text.Wrap );
			

//			FDisplay.begin( SSD1306_SWITCHCAPVCC, Address, false );
//			FValue = InitialValue;
//			inherited::SystemInit();

//			drawPixel( 20, 20, WHITE );

			for( int i = 0; i < FElements.size(); ++ i )
				FElements[ i ]->Render( false );

			display();
		}

		void DoRefreshReceived( void * )
		{
			display();
		}

		public:
			DisplaySSD1306I2C( int16_t AWidth, int16_t AHeight, const unsigned char * AFont ) :
				Graphics( AWidth, AHeight, AFont )
			{
				buffer = new uint8_t[ AWidth * AHeight / 8 ];
				ClearScreen( BacgroundColor );
				RefreshInputPin.SetCallback( MAKE_CALLBACK( DisplaySSD1306I2C::DoRefreshReceived ));
//				memset(buffer, 0, ( AWidth * AHeight / 8));
			}

	};
//---------------------------------------------------------------------------
}

#endif
