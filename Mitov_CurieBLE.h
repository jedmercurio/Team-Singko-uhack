////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_CURIE_BLE_h
#define _MITOV_CURIE_BLE_h

#include <Mitov.h>
#include <CurieBLE.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	class Arduino101UpdateIntf
	{
	public:
		virtual void UpdateValues() = 0;
	};
//---------------------------------------------------------------------------
	class Arduino101CurieBLE : public OpenWire::Component
	{
		typedef OpenWire::Component inherited;

	public:
		OpenWire::SourcePin ConnectedOutputPin;
		OpenWire::SourcePin MACAddressOutputPin;

	public:
		String	LocalName;
		String	DeviceName;

	public:
		BLEPeripheral	FPeripheral;
		bool			FAdvertised;

	public:
		void RegisterUpdateElement( Arduino101UpdateIntf *AElement )
		{
			FElements.push_back( AElement );
		}

	protected:
		Mitov::SimpleList<Arduino101UpdateIntf *>	FElements;
		bool	FConnected = false;

	protected:
		virtual void SystemLoopBegin( unsigned long currentMicros ) override
		{
			inherited::SystemLoopBegin( currentMicros );
			BLECentral ACentral = FPeripheral.central();
			if( FConnected )
			{
				if( !ACentral )
				{
					FConnected = false;
					ConnectedOutputPin.Notify( &FConnected );
				}
			}
			else
			{
				if( ACentral )
				{
					FConnected = true;
					ConnectedOutputPin.Notify( &FConnected );
					String AAddress = ACentral.address();
					MACAddressOutputPin.SendValue( AAddress );
				}
			}

			if( FConnected )
			{
				for( int i = 0; i < FElements.size(); ++ i )
					FElements[ i ]->UpdateValues();
			}
		}

		virtual void SystemInit() override
		{
			inherited::SystemInit();
			if( LocalName != "" )
				FPeripheral.setLocalName( LocalName.c_str() );

			if( DeviceName != "" )
				FPeripheral.setDeviceName( DeviceName.c_str() );
		}

		virtual void SystemStart() override
		{
			inherited::SystemStart();
			FPeripheral.begin();
		}
	};
//---------------------------------------------------------------------------
	class Arduino101BluetoothService : public OpenWire::Component
	{
		typedef OpenWire::Component inherited;

	public:
		String	UUID;
		bool	Advertise = false;

	public:
		Arduino101CurieBLE &FOwner;

	protected:
		BLEService	*FService;

	protected:
		virtual void SystemInit() override
		{
			inherited::SystemInit();
			FService = new BLEService( UUID.c_str() );
			FOwner.FPeripheral.addAttribute( *FService );
			if( Advertise )
				if( ! FOwner.FAdvertised )
				{
					FOwner.FPeripheral.setAdvertisedServiceUuid( FService->uuid());
					FOwner.FAdvertised = true;
				}
		}

	public:
		Arduino101BluetoothService( Arduino101CurieBLE &AOwner ) :
			FOwner( AOwner )
		{
		}
	};
//---------------------------------------------------------------------------
	template<typename T_CHARACT> class Arduino101BluetoothBasicCharacteristic : public OpenWire::Component
	{
		typedef OpenWire::Component inherited;

	public:
		String	UUID;

	protected:
		Arduino101BluetoothService &FOwner;
		T_CHARACT	*FCharacteristic;

	public:
		Arduino101BluetoothBasicCharacteristic( Arduino101BluetoothService &AOwner ) :
			FOwner( AOwner )
		{
		}
	};
//---------------------------------------------------------------------------
	template<typename T, typename T_CHARACT> class Arduino101BluetoothTypedBasicWriteCharacteristic : public Arduino101BluetoothBasicCharacteristic<T_CHARACT>, public Arduino101UpdateIntf
	{
		typedef Arduino101BluetoothBasicCharacteristic<T_CHARACT> inherited;

	public:
		OpenWire::TypedSourcePin<T>	OutputPin;

	public:
		T	InitialValue;
		bool	BigEndian = false;

	public:
		virtual void UpdateValues()
		{
			if( inherited::FCharacteristic->written() )
				OutputPin.SetValue( inherited::FCharacteristic->value(), true );

		}

	public:
		Arduino101BluetoothTypedBasicWriteCharacteristic( Arduino101BluetoothService &AOwner ) :
			inherited( AOwner )
		{
			AOwner.FOwner.RegisterUpdateElement( this );
		}

	};
//---------------------------------------------------------------------------
	template<typename T, typename T_CHARACT> class Arduino101BluetoothTypedWriteCharacteristic : public Arduino101BluetoothTypedBasicWriteCharacteristic<T, T_CHARACT>
	{
		typedef Arduino101BluetoothTypedBasicWriteCharacteristic<T, T_CHARACT> inherited;

	protected:
		virtual void SystemInit() override
		{
			inherited::SystemInit();
			inherited::FCharacteristic = new T_CHARACT( inherited::UUID.c_str(), BLEWrite );
			inherited::FOwner.FOwner.FPeripheral.addAttribute( *inherited::FCharacteristic );
			if( inherited::BigEndian )
				inherited::FCharacteristic->setValueBE( inherited::InitialValue );

			else
				inherited::FCharacteristic->setValueLE( inherited::InitialValue );

			inherited::OutputPin.SetValue( inherited::InitialValue, false );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	template<typename T, typename T_CHARACT> class Arduino101BluetoothTypedReadCharacteristic : public Arduino101BluetoothBasicCharacteristic<T_CHARACT>
	{
		typedef Arduino101BluetoothBasicCharacteristic<T_CHARACT> inherited;

	public:
		OpenWire::SinkPin	InputPin;

	public:
		T	InitialValue;
		bool	Notify : 1;
		bool	BigEndian : 1;

	protected:
		virtual void SystemInit() override
		{
			inherited::SystemInit();
			inherited::FCharacteristic = new T_CHARACT( inherited::UUID.c_str(), Notify ? ( BLERead | BLENotify ) : BLERead );
			inherited::FOwner.FOwner.FPeripheral.addAttribute( *inherited::FCharacteristic );
			if( BigEndian )
				inherited::FCharacteristic->setValueBE( InitialValue );

			else
				inherited::FCharacteristic->setValueLE( InitialValue );

		}

	protected:
		void DoDataReceive( void *_Data )
		{
			T AValue = *(T*)_Data;
			if( AValue != inherited::FCharacteristic->value() )
			{
				if( BigEndian )
					inherited::FCharacteristic->setValueBE( AValue );

				else
					inherited::FCharacteristic->setValueLE( AValue );
			}
		}

	public:
		Arduino101BluetoothTypedReadCharacteristic( Arduino101BluetoothService &AOwner ) :
			inherited( AOwner ),
			Notify( false ),
			BigEndian( false )
		{
			InputPin.SetCallback( MAKE_CALLBACK( Arduino101BluetoothTypedReadCharacteristic::DoDataReceive ));
		}

	};
//---------------------------------------------------------------------------
	template<typename T, typename T_CHARACT> class Arduino101BluetoothTypedReadWriteCharacteristic : public Arduino101BluetoothTypedBasicWriteCharacteristic<T, T_CHARACT>
	{
		typedef Arduino101BluetoothTypedBasicWriteCharacteristic<T, T_CHARACT> inherited;

	public:
		OpenWire::SinkPin	InputPin;

	protected:
		bool	Notify = false;

	protected:
		void DoDataReceive( void *_Data )
		{
			T AValue = *(T*)_Data;
			if( AValue != inherited::FCharacteristic->value() )
			{
				if( inherited::BigEndian )
					inherited::FCharacteristic->setValueBE( AValue );

				else
					inherited::FCharacteristic->setValueLE( AValue );
			}
		}

	protected:
		virtual void SystemInit() override
		{
			inherited::SystemInit();
			inherited::FCharacteristic = new T_CHARACT( inherited::UUID.c_str(), Notify ? ( BLERead | BLENotify | BLEWrite ) : BLERead | BLEWrite );
			inherited::FOwner.FOwner.FPeripheral.addAttribute( *inherited::FCharacteristic );
			if( inherited::BigEndian )
				inherited::FCharacteristic->setValueBE( inherited::InitialValue );

			else
				inherited::FCharacteristic->setValueLE( inherited::InitialValue );

			inherited::OutputPin.SetValue( inherited::InitialValue, false );
		}

	public:
		Arduino101BluetoothTypedReadWriteCharacteristic( Arduino101BluetoothService &AOwner ) :
			inherited( AOwner )
		{
			InputPin.SetCallback( MAKE_CALLBACK( Arduino101BluetoothTypedReadWriteCharacteristic::DoDataReceive ));
		}

	};
//---------------------------------------------------------------------------
	class Arduino101BluetoothBinaryBasicWriteCharacteristic : public Arduino101BluetoothBasicCharacteristic<BLECharacteristic>, public Arduino101UpdateIntf
	{
		typedef Arduino101BluetoothBasicCharacteristic<BLECharacteristic> inherited;

	public:
		OpenWire::SourcePin	OutputPin;

	public:
		Mitov::Bytes	InitialValue;

	public:
		virtual void UpdateValues()
		{
			if( inherited::FCharacteristic->written() )
				OutputPin.SendValue( Mitov::TDataBlock( inherited::FCharacteristic->valueLength(), inherited::FCharacteristic->value() ) );

		}

	public:
		Arduino101BluetoothBinaryBasicWriteCharacteristic( Arduino101BluetoothService &AOwner ) :
			inherited( AOwner )
		{
			AOwner.FOwner.RegisterUpdateElement( this );
		}
	};
//---------------------------------------------------------------------------
	template<int C_MAX_SIZE> class Arduino101BluetoothBinaryWriteCharacteristic : public Arduino101BluetoothBinaryBasicWriteCharacteristic
	{
		typedef Arduino101BluetoothBinaryBasicWriteCharacteristic inherited;

	protected:
		virtual void SystemInit() override
		{
			inherited::SystemInit();
			inherited::FCharacteristic = new BLECharacteristic( inherited::UUID.c_str(), BLEWrite, C_MAX_SIZE );
			inherited::FOwner.FOwner.FPeripheral.addAttribute( *inherited::FCharacteristic );
			inherited::FCharacteristic->setValue( inherited::InitialValue._Bytes, inherited::InitialValue._BytesSize );
//			inherited::OutputPin.SendValue( Mitov::TDataBlock( inherited::InitialValue._BytesSize, inherited::InitialValue._Bytes ) );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	template<int C_MAX_SIZE> class Arduino101BluetoothBinaryReadCharacteristic : public Arduino101BluetoothBasicCharacteristic<BLECharacteristic>
	{
		typedef Arduino101BluetoothBasicCharacteristic<BLECharacteristic> inherited;

	public:
		OpenWire::SinkPin	InputPin;

	public:
		Mitov::Bytes	InitialValue;
		bool	Notify = false;

	protected:
		virtual void SystemInit() override
		{
			inherited::SystemInit();
			inherited::FCharacteristic = new BLECharacteristic( inherited::UUID.c_str(), Notify ? ( BLERead | BLENotify ) : BLERead, C_MAX_SIZE );
			inherited::FOwner.FOwner.FPeripheral.addAttribute( *inherited::FCharacteristic );
			inherited::FCharacteristic->setValue( InitialValue._Bytes, InitialValue._BytesSize );
		}

	protected:
		void DoDataReceive( void *_Data )
		{
			Mitov::TDataBlock ABlock = *(Mitov::TDataBlock *)_Data;
			if( ABlock.Size == inherited::FCharacteristic->valueLength() )
				if( memcmp( ABlock.Data, inherited::FCharacteristic->value(), ABlock.Size ) == 0 )
					return;

			inherited::FCharacteristic->setValue( ABlock.Data, ABlock.Size );
		}

	public:
		Arduino101BluetoothBinaryReadCharacteristic( Arduino101BluetoothService &AOwner ) :
			inherited( AOwner )
		{
			InputPin.SetCallback( MAKE_CALLBACK( Arduino101BluetoothBinaryReadCharacteristic::DoDataReceive ));
		}
	};
//---------------------------------------------------------------------------
	template<int C_MAX_SIZE> class Arduino101BluetoothBinaryReadWriteCharacteristic : public Arduino101BluetoothBinaryBasicWriteCharacteristic
	{
		typedef Arduino101BluetoothBinaryBasicWriteCharacteristic inherited;

	public:
		OpenWire::SinkPin	InputPin;

	protected:
		bool	Notify = false;

	protected:
		void DoDataReceive( void *_Data )
		{
			Mitov::TDataBlock ABlock = *(Mitov::TDataBlock *)_Data;
			if( ABlock.Size == inherited::FCharacteristic->valueLength() )
				if( memcmp( ABlock.Data, inherited::FCharacteristic->value(), ABlock.Size ) == 0 )
					return;

			inherited::FCharacteristic->setValue( ABlock.Data, ABlock.Size );
		}

	protected:
		virtual void SystemInit() override
		{
			inherited::SystemInit();
			inherited::FCharacteristic = new BLECharacteristic( inherited::UUID.c_str(), Notify ? ( BLERead | BLENotify | BLEWrite ) : BLERead | BLEWrite, C_MAX_SIZE );
			inherited::FOwner.FOwner.FPeripheral.addAttribute( *inherited::FCharacteristic );
			inherited::FCharacteristic->setValue( inherited::InitialValue._Bytes, inherited::InitialValue._BytesSize );
//			inherited::OutputPin.SetValue( inherited::InitialValue, false );
		}

	public:
		Arduino101BluetoothBinaryReadWriteCharacteristic( Arduino101BluetoothService &AOwner ) :
			inherited( AOwner )
		{
			InputPin.SetCallback( MAKE_CALLBACK( Arduino101BluetoothBinaryReadWriteCharacteristic::DoDataReceive ));
		}
	};
//---------------------------------------------------------------------------
	class Arduino101BluetoothTextBasicWriteCharacteristic : public Arduino101BluetoothBasicCharacteristic<BLECharacteristic>, public Arduino101UpdateIntf
	{
		typedef Arduino101BluetoothBasicCharacteristic<BLECharacteristic> inherited;

	public:
		OpenWire::SourcePin	OutputPin;

	public:
		String	InitialValue;

	public:
		virtual void UpdateValues()
		{
			if( inherited::FCharacteristic->written() )
			{
				OutputPin.SendValue<String>( StringEx( (const char*)inherited::FCharacteristic->value(), inherited::FCharacteristic->valueLength() ) );
			}
		}

	public:
		Arduino101BluetoothTextBasicWriteCharacteristic( Arduino101BluetoothService &AOwner ) :
			inherited( AOwner )
		{
			AOwner.FOwner.RegisterUpdateElement( this );
		}
	};
//---------------------------------------------------------------------------
	template<int C_MAX_SIZE> class Arduino101BluetoothTextWriteCharacteristic : public Arduino101BluetoothTextBasicWriteCharacteristic
	{
		typedef Arduino101BluetoothTextBasicWriteCharacteristic inherited;

	protected:
		virtual void SystemInit() override
		{
			inherited::SystemInit();
			inherited::FCharacteristic = new BLECharacteristic( inherited::UUID.c_str(), BLEWrite, C_MAX_SIZE );
			inherited::FOwner.FOwner.FPeripheral.addAttribute( *inherited::FCharacteristic );
			inherited::FCharacteristic->setValue( (unsigned char *)inherited::InitialValue.c_str(), inherited::InitialValue.length() );
			inherited::OutputPin.SendValue<String>( StringEx( (const char*)inherited::FCharacteristic->value(), inherited::FCharacteristic->valueLength() ) );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	template<int C_MAX_SIZE> class Arduino101BluetoothTextReadCharacteristic : public Arduino101BluetoothBasicCharacteristic<BLECharacteristic>
	{
		typedef Arduino101BluetoothBasicCharacteristic<BLECharacteristic> inherited;

	public:
		OpenWire::SinkPin	InputPin;

	public:
		String	InitialValue;
		bool	Notify = false;

	protected:
		virtual void SystemInit() override
		{
			inherited::SystemInit();
			inherited::FCharacteristic = new BLECharacteristic( inherited::UUID.c_str(), Notify ? ( BLERead | BLENotify ) : BLERead, C_MAX_SIZE );
			inherited::FOwner.FOwner.FPeripheral.addAttribute( *inherited::FCharacteristic );
			inherited::FCharacteristic->setValue( (unsigned char *)InitialValue.c_str(), InitialValue.length() );
		}

	protected:
		void DoDataReceive( void *_Data )
		{
			String AData = (char *)_Data;
			if( AData.length() == inherited::FCharacteristic->valueLength() )
				if( memcmp( AData.c_str(), inherited::FCharacteristic->value(), AData.length() ) == 0 )
					return;

			inherited::FCharacteristic->setValue( (unsigned char *)AData.c_str(), AData.length() );
		}

	public:
		Arduino101BluetoothTextReadCharacteristic( Arduino101BluetoothService &AOwner ) :
			inherited( AOwner )
		{
			InputPin.SetCallback( MAKE_CALLBACK( Arduino101BluetoothTextReadCharacteristic::DoDataReceive ));
		}
	};
//---------------------------------------------------------------------------
	template<int C_MAX_SIZE> class Arduino101BluetoothTextReadWriteCharacteristic : public Arduino101BluetoothTextBasicWriteCharacteristic
	{
		typedef Arduino101BluetoothTextBasicWriteCharacteristic inherited;

	public:
		OpenWire::SinkPin	InputPin;

	protected:
		bool	Notify = false;

	protected:
		void DoDataReceive( void *_Data )
		{
			String AData = (char *)_Data;
			if( AData.length() == inherited::FCharacteristic->valueLength() )
				if( memcmp( AData.c_str(), inherited::FCharacteristic->value(), AData.length() ) == 0 )
					return;

			inherited::FCharacteristic->setValue( (unsigned char *)AData.c_str(), AData.length() );
		}

	protected:
		virtual void SystemInit() override
		{
			inherited::SystemInit();
			inherited::FCharacteristic = new BLECharacteristic( inherited::UUID.c_str(), Notify ? ( BLERead | BLENotify | BLEWrite ) : BLERead | BLEWrite, C_MAX_SIZE );
			inherited::FOwner.FOwner.FPeripheral.addAttribute( *inherited::FCharacteristic );
			inherited::FCharacteristic->setValue( (unsigned char *)inherited::InitialValue.c_str(), inherited::InitialValue.length() );
			inherited::OutputPin.SendValue<String>( StringEx( (const char*)inherited::FCharacteristic->value(), inherited::FCharacteristic->valueLength() ) );
		}

	public:
		Arduino101BluetoothTextReadWriteCharacteristic( Arduino101BluetoothService &AOwner ) :
			inherited( AOwner )
		{
			InputPin.SetCallback( MAKE_CALLBACK( Arduino101BluetoothTextReadWriteCharacteristic::DoDataReceive ));
		}
	};
//---------------------------------------------------------------------------
}

#endif
