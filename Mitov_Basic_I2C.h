////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_BASIC_I2C_h
#define _MITOV_BASIC_I2C_h

#include <Mitov.h>
#include <Wire.h>

#ifndef BUFFER_LENGTH
	#define BUFFER_LENGTH 256
#endif

namespace Mitov
{
//	class I2C : public OpenWire::Component
	namespace I2C
	{
		const uint16_t	I2C_DEFAULT_READ_TIMEOUT	= 1000;
//	public:
		bool ReadBytes( uint8_t devAddr, uint8_t regAddr, uint8_t length, void *data, uint16_t timeout = I2C_DEFAULT_READ_TIMEOUT )
		{
			int8_t count = 0;
			uint32_t t1 = millis();

            // Arduino v1.0.1+, Wire library
            // Adds official support for repeated start condition, yay!

            // I2C/TWI subsystem uses internal buffer that breaks with large data requests
            // so if user requests more than BUFFER_LENGTH bytes, we have to do it in
            // smaller chunks instead of all at once
            for (uint8_t k = 0; k < length; k += MitovMin(length, uint8_t( BUFFER_LENGTH )))
			{
                Wire.beginTransmission(devAddr);
                Wire.write(regAddr);
                Wire.endTransmission();
                Wire.beginTransmission(devAddr);
                Wire.requestFrom(devAddr, (uint8_t)MitovMin(length - k, int( BUFFER_LENGTH )));

				regAddr += BUFFER_LENGTH;
        
                for (; Wire.available() && (timeout == 0 || millis() - t1 < timeout); count++)
                    ((uint8_t *)data )[count] = Wire.read();
        
                Wire.endTransmission();
            }

			return ( count == length );
		}

		bool ReadBytes_16bitAddress( uint8_t devAddr, bool ABigIndianAddr, uint16_t regAddr, uint8_t length, void *data, uint16_t timeout = I2C_DEFAULT_READ_TIMEOUT )
		{
			int8_t count = 0;
			uint32_t t1 = millis();

            // Arduino v1.0.1+, Wire library
            // Adds official support for repeated start condition, yay!

            // I2C/TWI subsystem uses internal buffer that breaks with large data requests
            // so if user requests more than BUFFER_LENGTH bytes, we have to do it in
            // smaller chunks instead of all at once
            for (uint8_t k = 0; k < length; k += MitovMin(length, uint8_t( BUFFER_LENGTH ))) 
			{
                Wire.beginTransmission(devAddr);
				if( ABigIndianAddr )
				{
					Wire.write(regAddr >> 8);
					Wire.write(regAddr & 0xFF);
				}
				else
				{
					Wire.write(regAddr & 0xFF);
					Wire.write(regAddr >> 8);
				}

                Wire.endTransmission();
                Wire.beginTransmission(devAddr);
                Wire.requestFrom(devAddr, (uint8_t)MitovMin(length - k, int( BUFFER_LENGTH )));

				regAddr += BUFFER_LENGTH;
        
                for (; Wire.available() && (timeout == 0 || millis() - t1 < timeout); count++)
                    ((uint8_t *)data )[count] = Wire.read();
        
                Wire.endTransmission();
            }

			return ( count == length );
		}

		void WriteByte( uint8_t devAddr, uint8_t regAddr, uint8_t AValue )
		{
//			Serial.print( "Address: " ); Serial.print( devAddr, HEX ); Serial.print( " Reg: " ); Serial.print( regAddr, HEX );  Serial.print( " = " ); Serial.println( AValue, BIN );
			Wire.beginTransmission( devAddr );
			Wire.write( regAddr );
			Wire.write( AValue );
			Wire.endTransmission();
		}

		void WriteByte_16bitAddress( uint8_t devAddr, bool ABigIndianAddr, uint16_t regAddr, uint8_t AValue )
		{
//			Serial.print( "Address: " ); Serial.print( devAddr, HEX ); Serial.print( " Reg: " ); Serial.print( regAddr, HEX );  Serial.print( " = " ); Serial.println( AValue, BIN );
			Wire.beginTransmission( devAddr );
			if( ABigIndianAddr )
			{
				Wire.write(regAddr >> 8);
				Wire.write(regAddr & 0xFF);
			}

			else
			{
				Wire.write(regAddr & 0xFF);
				Wire.write(regAddr >> 8);
			}

			Wire.write( AValue );
			Wire.endTransmission();
		}

		void WriteBytes_16bitAddress( uint8_t devAddr, bool ABigIndianAddr, uint16_t regAddr, uint8_t length, void *data )
		{
//			Serial.print( "Address: " ); Serial.print( devAddr, HEX ); Serial.print( " Reg: " ); Serial.print( regAddr, HEX );  Serial.print( " = " ); Serial.println( AValue, BIN );
			Wire.beginTransmission( devAddr );
			if( ABigIndianAddr )
			{
				Wire.write(regAddr >> 8);
				Wire.write(regAddr & 0xFF);
			}

			else
			{
				Wire.write(regAddr & 0xFF);
				Wire.write(regAddr >> 8);
			}

			Wire.write( (byte *)data, length );
			Wire.endTransmission();
		}
	};
//---------------------------------------------------------------------------
	class Basic_I2CChannel;
//---------------------------------------------------------------------------
	class Basic_MultiChannel_SourceI2C : public Mitov::EnabledComponent, public ClockingSupport
	{
		typedef Mitov::EnabledComponent inherited;

	public:
		bool	FModified = false;

	public:
		Mitov::SimpleList<Basic_I2CChannel *>	FChannels;

//	protected:
//		virtual void SystemInit();

//	protected:
//		virtual void DoClockReceive( void * );

	};
//---------------------------------------------------------------------------
	class Basic_I2CChannel : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink	inherited;

	public:
		float	FValue = 0.0f;
		float	FNewValue = 0.0f;

//	public:
//		virtual void InitChannel() {}
//		virtual void SendOutput() = 0;

	};
//---------------------------------------------------------------------------
	template<typename T_OWNER> class Basic_Typed_I2CChannel : public Mitov::Basic_I2CChannel
	{
		typedef Mitov::Basic_I2CChannel	inherited;

	public:
		float	InitialValue = 0.0f;

	protected:
		int		FIndex;

	protected:
		T_OWNER	&FOwner;

/*
	protected:
		virtual void DoReceive( void *_Data )
		{
			FNewValue = constrain( *((float *)_Data), 0, 1 );
			if( FNewValue == FValue )
				return;

			FOwner.FModified = true;

			if( FOwner.ClockInputPin.IsConnected() )
				FOwner.FModified = true;

			else
				SendOutput();

		}
*/

	public:
		Basic_Typed_I2CChannel( T_OWNER &AOwner, int AIndex ) :
			FOwner( AOwner ),
			FIndex( AIndex )
		{
			AOwner.FChannels.push_back( this );
		}

	};
//---------------------------------------------------------------------------
	class ArduinoI2CInput
	{
	public:
		const static bool Enabled = true;

	public:
		void Print( const String AValue )
		{
			Wire.write( AValue.c_str(), AValue.length());
			Wire.write( '\r' );
			Wire.write( '\n' );
		}

		void Print( float AValue )
		{
			char AText[ 16 ];
			dtostrf( AValue,  1, 2, AText );
			Print( String( AText ));
		}

		void Print( int32_t AValue )
		{
			char AText[ 16 ];
			itoa( AValue, AText, 10 );
			Print( String( AText ));
		}

		void Print( uint32_t AValue )
		{
			char AText[ 16 ];
			itoa( AValue, AText, 10 );
			Print( String( AText ));
		}

		void PrintChar( char AValue )
		{
			Wire.write( AValue );
		}

		void PrintChar( byte AValue )
		{
			Wire.write( AValue );
		}

		void Write( uint8_t *AData, uint32_t ASize )
		{
			Wire.write( AData, ASize );
		}

	};
//---------------------------------------------------------------------------
	class ArduinoI2COutput : public OpenWire::Component
	{
		typedef OpenWire::Component	inherited;

	public:
		static OpenWire::SourcePin	OutputPin;
		static OpenWire::SourcePin	RequestOutputPin;

	protected:
		static void receiveEvent(int howMany) 
		{
//			Serial.println( "Test1" );
			while( 1 < Wire.available())
			{ // loop through all but the last
//  				Serial.println( "Test2" );
				byte AByte = Wire.read();
				OutputPin.SendValue( Mitov::TDataBlock( 1, &AByte ));
			}

			byte AByte = Wire.read();
			OutputPin.SendValue( Mitov::TDataBlock( 1, &AByte ));
		}

		static void requestEvent()
		{
			RequestOutputPin.Notify( nullptr );
		}

	protected:
		virtual void SystemInit() override
		{
			inherited::SystemInit();
			Wire.onReceive( receiveEvent );
			Wire.onRequest( requestEvent );
		}

/*
	protected:
		virtual void SystemLoopBegin( unsigned long currentMicros ) override
		{
			if( Wire.available() )
			{
				int AData = Wire.read();
				if( AData >= 0 )
				{
					byte AByte = AData;
					OutputPin.SendValue( Mitov::TDataBlock( 1, &AByte ));
				}
			}
		}
*/
	};
//---------------------------------------------------------------------------
	OpenWire::SourcePin	ArduinoI2COutput::OutputPin;
	OpenWire::SourcePin	ArduinoI2COutput::RequestOutputPin;
//---------------------------------------------------------------------------
	class I2CDevice : public OpenWire::Object // : public Mitov::CommonSink
	{
//		typedef Mitov::CommonSink	inherited;

	public:
		uint8_t	Address = 0;

	public:
		const static bool Enabled = true;

	public:
	/*
		template<typename T> void Print( T AValue )
		{
		}
	*/
		void Print( const String AValue )
		{
			Wire.beginTransmission( Address );
			Wire.write( AValue.c_str(), AValue.length());
			Wire.write( '\r' );
			Wire.write( '\n' );
			Wire.endTransmission();
		}

		void Print( float AValue )
		{
			char AText[ 16 ];
			dtostrf( AValue,  1, 2, AText );
			Print( String( AText ));
		}

		void Print( int32_t AValue )
		{
			char AText[ 16 ];
			itoa( AValue, AText, 10 );
			Print( String( AText ));
		}

		void Print( uint32_t AValue )
		{
			char AText[ 16 ];
			itoa( AValue, AText, 10 );
			Print( String( AText ));
		}

		void PrintChar( char AValue )
		{
			Wire.beginTransmission( Address );
			Wire.write( AValue );
			Wire.endTransmission();
		}

		void PrintChar( byte AValue )
		{
			Wire.beginTransmission( Address );
			Wire.write( AValue );
			Wire.endTransmission();
		}

		void Write( uint8_t *AData, uint32_t ASize )
		{
			Wire.beginTransmission( Address );
			Wire.write( AData, ASize );
			Wire.endTransmission();
		}

//	protected:
//		virtual void DoReceive( void *_Data ) override
//		{
//			Wire.beginTransmission( Address );
//		}
	};
//---------------------------------------------------------------------------
	class I2CDeviceRequest : public Mitov::ClockingSupport
	{
	protected:
		I2CDevice &FOwner;

	public:
		OpenWire::SourcePin	OutputPin;

	public:
		uint8_t	Size = 1;

	protected:
		virtual void DoClockReceive( void *_Data )
		{
			Wire.requestFrom( FOwner.Address, Size );

			for( int i = 0; i < Size; ++i )
			{
				if( Wire.available() )
				{
					byte AByte = Wire.read();
//					Serial.println( AByte );
					OutputPin.SendValue( Mitov::TDataBlock( 1, &AByte ));
				}
			}
		}

	public:
		I2CDeviceRequest( I2CDevice &AOwner ) :
			FOwner( AOwner )
		{
		}

	};
//---------------------------------------------------------------------------
/*
	void Basic_MultiChannel_SourceI2C::DoClockReceive( void * )
	{
		if( ! FModified )
			return;

		for( int i =0; i < FChannels.size(); ++i )
			FChannels[ i ]->SendOutput();
	}
*/
//---------------------------------------------------------------------------
/*
	void Basic_MultiChannel_SourceI2C::SystemInit()
	{
		inherited::SystemInit();

		for( int i =0; i < FChannels.size(); ++i )
			FChannels[ i ]->InitChannel();
	}
*/
//---------------------------------------------------------------------------
}
#endif
