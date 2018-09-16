////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_MEMORY_h
#define _MITOV_MEMORY_h

#include <Mitov.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	class BasicMemoryElement;
//---------------------------------------------------------------------------
	class MemoryIntf
	{
	public:
		Mitov::SimpleList<BasicMemoryElement *>	FElements;

	public:
		virtual bool ReadData( uint32_t AAddress, uint32_t ASize, byte *AData ) = 0;
		virtual bool WriteData( uint32_t AAddress, uint32_t ASize, byte *AData ) = 0;

	};
//---------------------------------------------------------------------------
	class BasicMemoryElement
	{
	public:
		virtual void PopulateAddress( uint32_t &AAddress, byte &AOffset ) = 0;
		virtual void ProcessRead( MemoryIntf *AOwner ) = 0;
		virtual void ProcessWrite( MemoryIntf *AOwner ) = 0;
//		virtual void GetAddressAndSize( uint32_t &AAddress, &ASize ) = 0;
	};
//---------------------------------------------------------------------------
	template<typename T_PIN, typename T> class TypedMemoryElement : public BasicMemoryElement
	{
		typedef	BasicMemoryElement inherited;

	public:
		OpenWire::ValueSimpleClockedFlagSinkPin	RememberInputPin;
		OpenWire::ValueSimpleClockedFlagSinkPin	RecallInputPin;
		OpenWire::ValueSimpleModifiedSinkPin<T>	InputPin;
		OpenWire::SourcePin						OutputPin;

	protected:
		bool	FRememberRequested = false;
		uint32_t	FAddress;

	public:
		virtual void PopulateAddress( uint32_t &AAddress, byte &AOffset ) override
		{
			if( AOffset )
			{
				++ AAddress;
				AOffset = 0;
			}

			FAddress = AAddress;
			AAddress += sizeof( T );
		}

		virtual void ProcessRead( MemoryIntf *AOwner ) override
		{
			if( ! RecallInputPin.Clocked )
				return;

			RecallInputPin.Clocked = false;

			T_PIN AValue;
			if( AOwner->ReadData( FAddress, sizeof( T ), (byte *)&AValue ))
				OutputPin.Notify( &AValue );
		}

		virtual void ProcessWrite( MemoryIntf *AOwner ) override
		{
			if( ! RememberInputPin.Clocked )
				return;

			RememberInputPin.Clocked = false;

//			Serial.println( "WRITE" );

			AOwner->WriteData( FAddress, sizeof( T ), (byte *)&InputPin.Value );
		}

/*
		virtual void GetAddressAndSize( uint32_t &AAddress, &ASize ) override
		{
			AAddress = FAddress;
			ASize = sizeof( T );
		}
*/
	public:
		TypedMemoryElement( MemoryIntf &AOwner )
		{
			AOwner.FElements.push_back( this );
		}

	};
//---------------------------------------------------------------------------
	class DigitalMemoryElement : public TypedMemoryElement<bool>
	{
		typedef	TypedMemoryElement<bool> inherited;

	protected:
		byte	FMask;

	public:
		virtual void PopulateAddress( uint32_t &AAddress, byte &AOffset ) override
		{
			FMask = 1 << AOffset;
			FAddress = AAddress;
			if( ++AOffset >= 8 )
			{
				++ AAddress;
				AOffset = 0;
			}
		}

		virtual void ProcessRead( MemoryIntf *AOwner ) override
		{
			if( ! RecallInputPin.Clocked )
				return;

			RecallInputPin.Clocked = false;

//			Serial.print( "READ: " ); Serial.println( FAddress, HEX );

			byte AByteValue;
			if( AOwner->ReadData( FAddress, 1, &AByteValue ))
			{
				bool AValue = (( AByteValue & FMask ) != 0 );
				OutputPin.Notify( &AValue );
			}
		}

		virtual void ProcessWrite( MemoryIntf *AOwner ) override
		{
			if( ! RememberInputPin.Clocked )
				return;

			RememberInputPin.Clocked = false;

//			Serial.println( "WRITE" );

			byte AByteValue;
			if( AOwner->ReadData( FAddress, 1, &AByteValue ))
			{
				if( InputPin.Value )
					AByteValue |= FMask;

				else
					AByteValue &= ~FMask;

				AOwner->WriteData( FAddress, 1, &AByteValue );
			}
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
}

#endif
