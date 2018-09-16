////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_GPIO_PCF8574_h
#define _MITOV_GPIO_PCF8574_h

#include <Mitov.h>
#include <Mitov_Basic_GPIO.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	class GPIO_PCF8574 : public Mitov::BasicGPIO<OpenWire::Component>
	{
		typedef Mitov::BasicGPIO<OpenWire::Component> inherited;

	public:
		uint8_t	Address = 0x38;

	protected:
		uint8_t	FDirectionBits = 0xFF;
		uint8_t	FReadBits = 0;


	protected:
		void UpdateAll()
		{
			Wire.beginTransmission( Address );
			Wire.write( FDirectionBits );
			Wire.endTransmission();
		}

	public:
		virtual void PerformRead() override
		{
			if( ! FDirectionBits )
				return;

			Wire.requestFrom( Address, (uint8_t)1 );
			FReadBits = Wire.read();

			for( int i = 0; i < FChannels.size(); ++i )
				FChannels[ i ]->UpdateInput();

		}

	public:
		bool GetBitValue( uint8_t AIndex )
		{
			return( ( FReadBits & ( ((uint8_t)1 ) << AIndex )) != 0 );
		}

	public:
		void SetChannelValue( int AIndex, bool AValue )
		{
			if( AValue )
				FDirectionBits |= ( (uint8_t)1 ) << AIndex;

			else
				FDirectionBits &= ~( ((uint8_t)1 ) << AIndex );
		}

	protected:
		virtual void SystemStart()
		{
			inherited::SystemStart();

			UpdateAll();

			for( int i = 0; i < FChannels.size(); ++i )
				FChannels[ i ]->SendOutput();
		}

		virtual void SystemLoopUpdateHardware()
		{
			UpdateAll();
			inherited::SystemLoopUpdateHardware();
		}

	};
//---------------------------------------------------------------------------
	class GPIO_PCF8574_Channel : public OwnedBasicGPIOChannel<GPIO_PCF8574>
	{
		typedef OwnedBasicGPIOChannel<GPIO_PCF8574> inherited;
		
	public:
		virtual void UpdateInput()
		{			
			UpdateOutput( FOwner.GetBitValue( FIndex ));
		}

	protected:
        virtual void PinDirectionsInit()
        {
			if( FIsOutput )
				FOwner.SetChannelValue( FIndex, FValue );

			else
				FOwner.SetChannelValue( FIndex, true );

        }

		virtual void DoDataReceive( void * _Data ) override
		{
			bool AValue = *(bool *)_Data;
			if( FValue == AValue )
				return;

			FValue = AValue;
			if( FIsOutput )
				FOwner.SetChannelValue( FIndex, AValue );
		}

	public:
		GPIO_PCF8574_Channel( GPIO_PCF8574 &AOwner, int AIndex, bool AInitialValue, bool AIsOutput, bool AIsPullUp, bool AIsCombinedInOut ) :
			inherited( AOwner, AIndex, AInitialValue, AIsOutput, AIsPullUp, AIsCombinedInOut )
		{
			PinDirectionsInit();
		}

	};
//---------------------------------------------------------------------------
}

#endif
