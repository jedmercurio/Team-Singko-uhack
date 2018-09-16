////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_EEPROM_h
#define _MITOV_EEPROM_h

#include <Mitov.h>
#include <Mitov_Memory.h>
#include <EEPROM.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	class ArduinoEEPROM : public OpenWire::Component, public Mitov::MemoryIntf
	{
		typedef	OpenWire::Component	inherited;
		
	public:
		virtual bool ReadData( uint32_t AAddress, uint32_t ASize, byte *AData ) override
		{
			if( ASize == 0 )
				return	true;

	        for( int i = 0; i < ASize; ++i )
				*AData++ = EEPROM.read( AAddress++ );

			return true;
		}

		virtual bool WriteData( uint32_t AAddress, uint32_t ASize, byte *AData ) override
		{
#ifdef VISUINO_ESP8266
			EEPROM.begin( ASize );
	        for( int i = 0; i < ASize; ++i )
				EEPROM.write( AAddress++, *AData++ );

			EEPROM.commit();
#else
	        for( int i = 0; i < ASize; ++i )
				EEPROM.update( AAddress++, *AData++ );
#endif

			return true;
		}

	protected:
		virtual void SystemInit() override
		{
			inherited::SystemInit();

			uint32_t AAddress = 0;
			byte AOffset = 0;
			for( int i = 0; i < FElements.size(); ++ i )
				FElements[ i ]->PopulateAddress( AAddress, AOffset );

		}

		virtual void SystemLoopEnd() override
		{
			for( int i = 0; i < FElements.size(); ++ i )
				FElements[ i ]->ProcessRead( this );

//			inherited::SystemLoopEnd();
		}

		virtual void SystemLoopUpdateHardware() override
		{
			for( int i = 0; i < FElements.size(); ++ i )
				FElements[ i ]->ProcessWrite( this );

//			inherited::SystemLoopEnd();
		}

	};
//---------------------------------------------------------------------------
}

#endif
