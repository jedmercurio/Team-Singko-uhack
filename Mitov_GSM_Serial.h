////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_GSM_SERIAL_h
#define _MITOV_GSM_SERIAL_h

#include <Mitov.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	class MitovGSMSerial;
	class MitovGSMSerialVoiceCallFunction;
	class MitovGSMSerialSMSMessageReceivedFunction;
//---------------------------------------------------------------------------
	class MitovGSMSerialBasicFunction : public OpenWire::Component
	{
	public:
		MitovGSMSerial &FOwner;

	public:
		virtual void ElementSystemStart() {} // Make sure it is different than SystemStart
		virtual bool TryProcessRequestedInput( String ALine, bool &ALockInput, bool &AResponseCompleted ) { return false; }
		virtual bool TryProcessInput( String ALine, bool &ALockInput ) { return false; }

	public:
		MitovGSMSerialBasicFunction( MitovGSMSerial &AOwner );
	};
//---------------------------------------------------------------------------
	class MitovGSMSerialBasicExpectOKFunction : public MitovGSMSerialBasicFunction
	{
		typedef MitovGSMSerialBasicFunction inherited;

	protected:
		bool	FEmptyLineDetected = false;

	public:
		void Reset()
		{
			FEmptyLineDetected = false;
		}

	protected:
		virtual void OKReceived()
		{
		}

	public:
		virtual bool TryProcessRequestedInput( String ALine, bool &ALockInput, bool &AResponseCompleted ) override
		{
			Serial.println( "TryProcessRequestedInput" );
			Serial.println( ALine );
			if( FEmptyLineDetected )
			{
				ALine.trim();
				if( ALine == "OK" )
				{
					Serial.println( "OK Processed" );
	//					Serial.println( "ALockInput = false" );
					AResponseCompleted = true;
					FEmptyLineDetected = false;
					ALockInput = false;
					OKReceived();
					return true;
				}
			}

			else if( ALine == "" )
			{
				Serial.println( "FEmptyLineDetected" );
				FEmptyLineDetected = true;
				return true;
			}

			return false;
		}

	public:
		using inherited::inherited;


	};
//---------------------------------------------------------------------------
	class MitovGSMSerialPowerOnFunction : public MitovGSMSerialBasicExpectOKFunction
	{
		typedef MitovGSMSerialBasicExpectOKFunction inherited;

	protected:
		virtual void OKReceived() override;

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class MitovGSMSerial : public OpenWire::Component
	{
		typedef OpenWire::Component inherited;

	public:
		OpenWire::SourcePin	PowerOutputPin;

	public:
		bool	PowerOn = true;

	protected:
		bool	FInPowerCheckWait = false;
		bool	FInPowerSwitch = false;
		unsigned long FLastTime = 0;

	public:
		void SetPowerOn( bool AValue )
		{
			if( PowerOn == AValue )
				return;

			PowerOn = AValue;
			Serial.println( "SetPowerOn" );
			Serial.println( PowerOn );
			QueryPowerOn();
		}

	protected:
		Mitov::SimpleList<MitovGSMSerialBasicFunction *>	FFunctions;
		Mitov::SimpleList<MitovGSMSerialBasicFunction *>	FResponseHandlersQueue;
		Mitov::SimpleList<String>							FQueryQueue;

	public:
		void AddFunction( MitovGSMSerialBasicFunction *AFunction )
		{
			FFunctions.push_back( AFunction );
		}

	public:
		void SendQuery( String AQuery )
		{
			Serial.print( "QUERY : \"" );	Serial.print( AQuery ); Serial.println( "\"" );

			FStream.println( AQuery );
//			delay( 1000 );
		}

		void SendQueryRegisterResponse( MitovGSMSerialBasicFunction *ASender, String AQuery )
		{
			if( ( PowerOn || ( AQuery == "AT" ) ) && ( FResponseHandlersQueue.size() == 0 ) && ( !FInPowerSwitch ))
				SendQuery( AQuery );

			else
			{
				Serial.print( "ADD TO QUERY : \"" );	Serial.print( AQuery ); Serial.println( "\"" );
				FQueryQueue.push_back( AQuery );
			}
			
//			SendQuery( AQuery );
			FResponseHandlersQueue.push_back( ASender );
		}

		void AbortResponseHandler( MitovGSMSerialBasicFunction *ASender )
		{
			if( FLockRequestedInputIndex )
				if( FResponseHandlersQueue[ FLockRequestedInputIndex - 1 ] == ASender)
					FLockRequestedInputIndex = 0;

			Serial.println( "FResponseHandlersQueue.erase" );
			FResponseHandlersQueue.erase( ASender );
		}

	public:
		Stream &FStream;

		MitovGSMSerialBasicExpectOKFunction	FExpectOKReply;

		MitovGSMSerialPowerOnFunction		FPowerOnFunction;

	protected:
		char		FBuffer[ 256 ];
		uint8_t		FIndex = 0;
		uint16_t	FLockInputIndex = 0;
		uint16_t	FLockRequestedInputIndex = 0;

	protected:
		void ReadSerial()
		{
//			if( FInPowerSwitch )
//				return;

			int AChar = FStream.read();
//			Serial.print( AChar );
			if( AChar < 0 )
				return;

//			Serial.print( (char)AChar );
//			if( AChar < ' ' )
//				Serial.println( AChar );

			if( AChar == 13 )
				return;

			if( AChar != 10 )
			{
				FBuffer[ FIndex ++ ] = AChar;
				if( FIndex < 255 )
					return;
			}

//			Serial.println( "TEST!!!" );
//			Serial.println( "" );
//			Serial.println( FIndex );

			FBuffer[ FIndex ] = '\0';
			FIndex = 0;

			String AString = FBuffer;

			Serial.print( "LINE: " ); Serial.println( AString );

//			Serial.print( "QUEUE: " ); Serial.println( FResponseHandlersQueue.size() );

			bool	ALockInput;
			bool	AResponseCompleted = false;
//			Serial.print( "FLockInputIndex : " ); Serial.println( FLockInputIndex );
			if( FLockRequestedInputIndex )
			{
				ALockInput = true;
				if( FResponseHandlersQueue[ FLockRequestedInputIndex - 1 ]->TryProcessRequestedInput( AString, ALockInput, AResponseCompleted ))
				{
					if( AResponseCompleted )
					{
						Serial.println( "Queue Delete 1" );
//						Serial.println( ALockInput );
//						Serial.print( "RESP_QUEUE: " ); Serial.println( FResponseHandlersQueue.size() );
						FResponseHandlersQueue.Delete( FLockRequestedInputIndex - 1 );
//						Serial.print( "RESP_QUEUE: " ); Serial.println( FResponseHandlersQueue.size() );
						if( FQueryQueue.size() )
						{
//							Serial.print( "SEND_QUERY: " ); Serial.println( FQueryQueue.size() );
							String ACommand = FQueryQueue[ 0 ];
//							Serial.print( "ESTRACT_QUERY: " ); Serial.println( ACommand );
							FQueryQueue.pop_front();
							SendQuery( ACommand );
//							Serial.print( "SEND_QUERY: " ); Serial.println( FQueryQueue.size() );
						}
					}

					if( ! ALockInput )
						FLockRequestedInputIndex = 0;
				}

				return;
			}

			ALockInput = false;
			AResponseCompleted = false;
			for( int i = 0; i < FResponseHandlersQueue.size(); ++i )
				if( FResponseHandlersQueue[ i ]->TryProcessRequestedInput( AString, ALockInput, AResponseCompleted ))
				{
					if( ALockInput )
						FLockRequestedInputIndex = i + 1;

					if( AResponseCompleted )
					{
						Serial.println( "Queue Delete 2" );
//						Serial.print( "RESP_QUEUE: " ); Serial.println( FResponseHandlersQueue.size() );
						FResponseHandlersQueue.Delete( i );
						Serial.print( "RESP_QUEUE: " ); Serial.println( FResponseHandlersQueue.size() );
						if( FQueryQueue.size() )
						{
//							Serial.print( "SEND_QUERY: " ); Serial.println( FQueryQueue.size() );
							String ACommand = FQueryQueue[ 0 ];
							Serial.print( "ESTRACT_QUERY: " ); Serial.println( ACommand );
							FQueryQueue.pop_front();
							SendQuery( ACommand );
							Serial.print( "SEND_QUERY: " ); Serial.println( FQueryQueue.size() );
						}
					}

					return;
				}

			if( FLockInputIndex )
			{
//				Serial.println( "FLockInputIndex" );
				ALockInput = true;
				FFunctions[ FLockInputIndex - 1 ]->TryProcessInput( AString, ALockInput );
				if( ! ALockInput )
					FLockInputIndex = 0;

				return;
			}

//			Serial.println( "*****" );
			ALockInput = false;
			for( int i = 0; i < FFunctions.size(); ++i )
				if( FFunctions[ i ]->TryProcessInput( AString, ALockInput ))
				{
					if( ALockInput )
						FLockInputIndex = i + 1;

					return;
				}
/*
			ALine.trim();
			if( ALine == "RDY" )
			{
				return true;
			}

			if( ALine == "NORMAL POWER DOWN" )
			{
				return true;
			}

			if( ALine == "Call Ready" )
			{
			}

*/
		}

	public:
		void PulsePower()
		{
			FInPowerSwitch = true;
			PowerOutputPin.SendValue( true );
			FLastTime = micros();
		}

		void PowerOKReceived()
		{
			Serial.println( "PowerOKReceived()" );
			FInPowerCheckWait = false;
			if( ! PowerOn )
				PulsePower();

		}

	protected:
		void QueryPowerOn()
		{
			Serial.print( "FInPowerSwitch = " ); Serial.println( FInPowerSwitch );

			if( FInPowerSwitch )
				return;

			Serial.println( "AT..." );
			SendQueryRegisterResponse( &FPowerOnFunction, "AT" );
			FLastTime = micros();
			FInPowerCheckWait = true;
		}

	protected:
		virtual void SystemLoopBegin( unsigned long currentMicros ) override
		{
			inherited::SystemLoopBegin( currentMicros );
			if( FInPowerCheckWait )
			{
				if( currentMicros - FLastTime >= 2000000 )
				{
					FInPowerCheckWait = false;
					FPowerOnFunction.Reset();
					AbortResponseHandler( &FPowerOnFunction );
					Serial.println( "FInPowerCheckWait" );
					if( PowerOn )
						PulsePower();

//					PowerOutputPin.SendValue( false );
				}
			}

			else if( FInPowerSwitch )
			{
				if( currentMicros - FLastTime >= 2000000 )
				{
					Serial.println( "FInPowerSwitch" );
					FInPowerSwitch = false;	
					PowerOutputPin.SendValue( false );
				}
			}

			ReadSerial();
			if( FResponseHandlersQueue.size() == 0 )
				if( FQueryQueue.size() )
				{
					String ACommand = FQueryQueue[ 0 ];
					FQueryQueue.pop_front();
					SendQuery( ACommand );
				}
		}

		virtual void SystemStart() override
		{
			PowerOutputPin.SendValue( false );
			QueryPowerOn();

			for( int i = 0; i < FFunctions.size(); ++i )
				FFunctions[ i ]->ElementSystemStart();
		}

	public:
		MitovGSMSerial( Mitov::BasicSerialPort &ASerial ) :
			FStream( ASerial.GetStream() ),
			FExpectOKReply( *this ),
			FPowerOnFunction( *this )
		{
		}

	};
//---------------------------------------------------------------------------
	class TArduinoGSMReceivingVoiceCallAutoAnswer
	{
	public:
		bool Enabled : 1;
		uint8_t NumberRings : 7;

	public:
		TArduinoGSMReceivingVoiceCallAutoAnswer() :
			Enabled( false ),
			NumberRings( 1 )
		{
		}
	};
//---------------------------------------------------------------------------
    class MitovGSMSerialReceivingVoiceCall
	{
		typedef MitovGSMSerialBasicFunction inherited;

	public:
		OpenWire::SourcePin	CallingOutputPin;
		OpenWire::SourcePin	NumberOutputPin;
		OpenWire::SourcePin	AddressTypeOutputPin;
		OpenWire::SourcePin	SubAddressOutputPin;
		OpenWire::SourcePin	SubAddressTypeOutputPin;
		OpenWire::SourcePin	PhoneBookAddressOutputPin;
		OpenWire::SinkPin	AnswerInputPin;

	public:
		TArduinoGSMReceivingVoiceCallAutoAnswer	AutoAnswer;

	protected:
		uint8_t	FRingCount = 0;

	public:
		void Ringing();

		inline void ClearRingCount()
		{
			FRingCount = 0;
		}

	protected:
		MitovGSMSerialVoiceCallFunction	&FOwner;

	public:
		MitovGSMSerialReceivingVoiceCall( MitovGSMSerialVoiceCallFunction &AOwner );

	};
//---------------------------------------------------------------------------
	class MitovGSMSerialBasicVoiceFunction : public MitovGSMSerialBasicFunction
	{
		typedef MitovGSMSerialBasicFunction inherited;

	protected:
		MitovGSMSerialVoiceCallFunction &FOwnerFunction;

	public:
		MitovGSMSerialBasicVoiceFunction( MitovGSMSerialVoiceCallFunction &AOwnerFunction );

	};
//---------------------------------------------------------------------------
	class MitovGSMSerialVoiceCallAnswerFunction : public MitovGSMSerialBasicVoiceFunction
	{
		typedef MitovGSMSerialBasicVoiceFunction inherited;

	protected:
		bool	FEmptyLineDetected = false;

	public:
		void Send()
		{
			FOwner.SendQueryRegisterResponse( this, "ATA" );
		}

	public:
		virtual bool TryProcessRequestedInput( String ALine, bool &ALockInput, bool &AResponseCompleted ) override;

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	enum TArduinoGSMSerialVoiceModuleExistingCallMode {ccmDrop, ccmHold};
//---------------------------------------------------------------------------
	class MitovGSMSerialVoiceCallFunction : public MitovGSMSerialBasicFunction
	{
		typedef MitovGSMSerialBasicFunction inherited;

	public:
		OpenWire::SourcePin BusyOutputPin;
		OpenWire::SourcePin InUseOutputPin;

	protected:
		enum TState { sIdle, sCalling, sReceivingCall, sTalking };

	public:
		MitovGSMSerialReceivingVoiceCall *FReceivingCall = nullptr;

	protected:
		TState	FState = sIdle;

		MitovGSMSerialVoiceCallAnswerFunction	FVoiceCallAnswerFunction;

	protected:
		void SetState( TState AValue )
		{
			FState = AValue;
//			Serial.print( "STATE: " ); Serial.println( AValue );
		}

	public:
		void DoReceiveAnswer( void * )
		{
			FVoiceCallAnswerFunction.Send();
		}

	public:
		void CallAnswered()
		{
			InUseOutputPin.SendValue( true );
			SetState( sTalking );
		}

		void MakingCall()
		{
			SetState( sCalling );
		}

		void DropCall()
		{
			if( sIdle )
				return;

			FOwner.SendQueryRegisterResponse( &FOwner.FExpectOKReply, "ATH" );
		}

		void PlaceOnHold()
		{
			if( sIdle )
				return;

			FOwner.SendQueryRegisterResponse( &FOwner.FExpectOKReply, "AT+CHLD=2" ); // Place on Hold
		}

	public:
		virtual bool TryProcessInput( String ALine, bool &ALockInput ) override
		{
//			Serial.println( "????" );
//			Serial.println( ALine );
			ALine.trim();
//			Serial.println( "TEST3333" );
			if( ALine.startsWith( "+CLIP:" )) //"RDY" )
			{
				ALine.remove( 0, 6 );
				ALine.trim();
//				Serial.println( "TEST111" );
//				ProcessLine( ALine, false );
				if( FReceivingCall )
				{
					String ANumber;
					if( Func::ExtractOptionallyQuotedCommaText( ALine, ANumber ))
					{
						String AAddressType;
						if( Func::ExtractOptionallyQuotedCommaText( ALine, AAddressType ))
						{
							FReceivingCall->NumberOutputPin.SendValue( ANumber );
							FReceivingCall->AddressTypeOutputPin.SendValue<uint32_t>( AAddressType.toInt() );
							ALine.trim();
							String ASubAddress;
							if( Func::ExtractOptionallyQuotedCommaText( ALine, ASubAddress ))
							{
								String ASubAddressType;
								if( Func::ExtractOptionallyQuotedCommaText( ALine, ASubAddressType ))
								{
									FReceivingCall->SubAddressOutputPin.SendValue( ASubAddress );
									FReceivingCall->SubAddressTypeOutputPin.SendValue( ASubAddressType.toInt() );
									ALine.trim();

									String APhoneBookAddress;
									if( Func::ExtractOptionallyQuotedCommaText( ALine, APhoneBookAddress ))
										FReceivingCall->PhoneBookAddressOutputPin.SendValue( APhoneBookAddress );

									else
										FReceivingCall->PhoneBookAddressOutputPin.SendValue( "" );
								}
							}
							else
							{
								FReceivingCall->SubAddressOutputPin.SendValue( "" );
								FReceivingCall->SubAddressTypeOutputPin.SendValue( 0 );
								FReceivingCall->PhoneBookAddressOutputPin.SendValue( "" );
							}

						}
					}
				}

				InUseOutputPin.SendValue( true );
				SetState( sReceivingCall );
				return true;
			}

			if( ALine == "RING" )
			{
//				Serial.println( "RRRRR" );
				if( FReceivingCall )
					FReceivingCall->Ringing();
//					FReceivingCall->CallingOutputPin.Notify( nullptr );

				InUseOutputPin.SendValue( true );
				SetState( sReceivingCall );
				return true;
			}

			if( ALine == "BUSY" )
			{
				BusyOutputPin.Notify( nullptr );
				InUseOutputPin.SendValue( false );
				SetState( sIdle );
				if( FReceivingCall )
					FReceivingCall->ClearRingCount();

				return true;
			}

			if( ALine == "NO CARRIER" )
			{
				InUseOutputPin.SendValue( false );
				SetState( sIdle );
				if( FReceivingCall )
					FReceivingCall->ClearRingCount();

				return true;
			}

			if( ALine.startsWith( "+COLP:" ))
			{
				InUseOutputPin.SendValue( true );
				SetState( sTalking );
				return true;
			}

			return false;
		}

		virtual void ElementSystemStart() override
		{
			InUseOutputPin.SendValue( false );
		}

	public:
		MitovGSMSerialVoiceCallFunction( MitovGSMSerial &AOwner ) :
			inherited( AOwner ),
			FVoiceCallAnswerFunction( *this )
		{
		}

	};
//---------------------------------------------------------------------------
	class MitovVoiceModuleExpectOKFunction : public MitovGSMSerialBasicVoiceFunction, public ClockingSupport
	{
		typedef MitovGSMSerialBasicVoiceFunction inherited;

	protected:
		bool	FEmptyLineDetected = false;

	public:
		virtual bool TryProcessRequestedInput( String ALine, bool &ALockInput, bool &AResponseCompleted ) override
		{
			if( FEmptyLineDetected )
			{
				ALine.trim();
				if( ALine == "OK" )
				{
	//				Serial.println( "OK Processed" );
	//					Serial.println( "ALockInput = false" );
					AResponseCompleted = true;
					FEmptyLineDetected = false;
					ALockInput = false;
					return true;
				}
			}

			else if( ALine == "" )
			{
				FEmptyLineDetected = true;
				return true;
			}

			return false;
		}

	public:
		virtual void ElementSystemStart() override 
		{
			if( ! ClockInputPin.IsConnected() )
				DoClockReceive( nullptr );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class MitovVoiceModuleCallFunction : public MitovVoiceModuleExpectOKFunction
	{
		typedef MitovVoiceModuleExpectOKFunction inherited;

	public:
		String Number;
		TArduinoGSMSerialVoiceModuleExistingCallMode ExistingCallMode = ccmDrop;

	protected:
		virtual void DoClockReceive( void *_Data ) override
		{
			if( ExistingCallMode == ccmDrop )
				FOwnerFunction.DropCall();

			else
				FOwnerFunction.PlaceOnHold();

			FOwner.SendQueryRegisterResponse( this, String( "ATD" ) + Number + ";" );
			FOwnerFunction.MakingCall();
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class MitovGSMSerialVoiceSelectAudioChannelFunction : public MitovVoiceModuleExpectOKFunction
	{
		typedef MitovVoiceModuleExpectOKFunction inherited;

	public:
		uint8_t	Channel = 0;

	protected:
		virtual void DoClockReceive( void *_Data ) override
		{
			FOwner.SendQueryRegisterResponse( this, String( "AT+CHFA=" ) + Channel );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class MitovGSMSerialVoiceSetVolumeFunction : public MitovVoiceModuleExpectOKFunction
	{
		typedef MitovVoiceModuleExpectOKFunction inherited;

	public:
		uint8_t	Channel = 0;
		float	Volume = 0.5;

	protected:
		virtual void DoClockReceive( void *_Data ) override
		{
			int AVolume = ( Volume * 15 ) + 0.5;
			FOwner.SendQueryRegisterResponse( this, String( "AT+CMIC=" ) + Channel + "," + AVolume );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class MitovGSMSerialSMSSendMessageFunction : public MitovGSMSerialBasicFunction, public ClockingSupport
	{
		typedef MitovGSMSerialBasicFunction inherited;

	public:
		OpenWire::SourcePin	ReferenceOutputPin;

	public:
		String Address;
		String Text;

	protected:
		bool	FEmptyLineDetected = false;

	public:
		void SetText( String AValue )
		{
			if( Text == AValue )
				return;

			Text = AValue;

			if( ! ClockInputPin.IsConnected() )
				TrySendValue();

		}

	protected:
		bool FLocked = false;

	protected:
		void TrySendValue()
		{
			if( Address == "" )
				return;

			if( Text == "" )
				return;

//			Serial.println( "TEST555" );

			FOwner.SendQueryRegisterResponse( this, String( "AT+CMGS = \"" ) + Address + "\"" );
			FOwner.FStream.println( Text );
			FOwner.FStream.println((char)26);	//the ASCII code of the ctrl+z is 26
		}

	public:
		virtual bool TryProcessRequestedInput( String ALine, bool &ALockInput, bool &AResponseCompleted ) override
		{
			if( FLocked )
			{
				if( FEmptyLineDetected )
				{
					ALine.trim();
					if( ALine == "OK" )
					{
		//				Serial.println( "OK Processed" );
		//					Serial.println( "ALockInput = false" );
						AResponseCompleted = true;
						FEmptyLineDetected = false;
						ALockInput = false;
						return true;
					}
				}

				else if( ALine == "" )
				{
					FEmptyLineDetected = true;
					return true;
				}

				return false;
			}

			if( ALine.startsWith( "+CMGS:" ) )
			{
				ALine.remove( 0, 6 );
				ALine.trim();
				uint32_t AReference = ALine.toInt();

				ReferenceOutputPin.Notify( &AReference );

				ALockInput = true;
				FLocked = true;
				return true;
			}

			return false;
		}

	protected:
		virtual void DoClockReceive( void *_Data ) override
		{
			TrySendValue();
		}

		void DoInputChange( void *_Data )
		{
			if( ClockInputPin.IsConnected() )
				return;

			TrySendValue();
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class MitovGSMSerialSMSSetModeFunction : public MitovGSMSerialBasicExpectOKFunction, public ClockingSupport
	{
		typedef MitovGSMSerialBasicExpectOKFunction inherited;

	public:
		bool	PDUMode : 1;

	protected:
		bool	FEmptyLineDetected : 1;

	public:
		virtual bool TryProcessRequestedInput( String ALine, bool &ALockInput, bool &AResponseCompleted ) override
		{
			if( FEmptyLineDetected )
			{
				ALine.trim();
				if( ALine == "OK" )
				{
	//				Serial.println( "OK Processed" );
	//					Serial.println( "ALockInput = false" );
					AResponseCompleted = true;
					FEmptyLineDetected = false;
					ALockInput = false;
					return true;
				}
			}

			else if( ALine == "" )
			{
				FEmptyLineDetected = true;
				return true;
			}

			return false;
		}

	protected:
		virtual void DoClockReceive( void *_Data ) override
		{
			FOwner.SendQueryRegisterResponse( this, String( "AT+CMGF=" ) + ( ( PDUMode ) ? "0" : "1" ) );
		}

	public:
		virtual void ElementSystemStart() override 
		{
//			inherited::SystemStart();
//			Serial.println( "CLOCK!" );
			if( ! ClockInputPin.IsConnected() )
				DoClockReceive( nullptr );

		}

	public:
		MitovGSMSerialSMSSetModeFunction( MitovGSMSerial &AOwner ) :
			inherited( AOwner ),
			PDUMode( false ),
			FEmptyLineDetected( false )
		{
		}

	};
//---------------------------------------------------------------------------
	class MitovGSMSerialSMSGetModeFunction : public MitovGSMSerialBasicFunction, public ClockingSupport
	{
		typedef MitovGSMSerialBasicFunction inherited;

	public:
		OpenWire::SourcePin	InPDUModeOutputPin;

	protected:
		bool	FLocked : 1;
		bool	FEmptyLineDetected : 1;

	protected:
		virtual void DoClockReceive( void *_Data ) override
		{
			FOwner.SendQueryRegisterResponse( this, "AT+CMGF?" );
		}

	public:
		virtual bool TryProcessRequestedInput( String ALine, bool &ALockInput, bool &AResponseCompleted ) override
		{
			if( FLocked )
			{
				if( FEmptyLineDetected )
				{
					ALine.trim();
					if( ALine == "OK" )
					{
		//				Serial.println( "OK Processed" );
		//					Serial.println( "ALockInput = false" );
						AResponseCompleted = true;
						FEmptyLineDetected = false;
						ALockInput = false;
						return true;
					}
				}

				else if( ALine == "" )
				{
					FEmptyLineDetected = true;
					return true;
				}
			}

			if( ALine.startsWith( "+CMGF:" ) )
			{
				ALine.remove( 0, 6 );
				ALine.trim();
				uint8_t AReference = ALine.toInt();

				InPDUModeOutputPin.SendValue<bool>( AReference == 0 );

				ALockInput = true;
				FLocked = true;
				return true;
			}

			return false;
		}

	public:
		virtual void ElementSystemStart() override
		{
//			inherited::SystemStart();
			if( ! ClockInputPin.IsConnected() )
				DoClockReceive( nullptr );

		}

	public:
		MitovGSMSerialSMSGetModeFunction( MitovGSMSerial &AOwner ) :
			inherited( AOwner ),
			FLocked( false ),
			FEmptyLineDetected( false )
		{
		}

	};
//---------------------------------------------------------------------------
	class MitovGSMSerialSMSBasicMessageFunction : public MitovGSMSerialBasicFunction
	{
		typedef MitovGSMSerialBasicFunction inherited;

	protected:
		bool FLocked = false;

	protected:
		virtual void ProcessLine( String ALine, bool AIsSecondLine ) {}

	public:
		virtual bool TryProcessInput( String ALine, bool &ALockInput ) override
		{
			if( FLocked )
			{
//				Serial.println( ALine );
				ProcessLine( ALine, true );
				ALockInput = false;
				FLocked = false;

				return true;
			}

			ALine.trim();
//			Serial.println( "TEST3333" );
			if( ALine.startsWith( "+CMT:" )) //"RDY" )
			{
//				Serial.println( "TEST111" );
				ProcessLine( ALine, false );

				FLocked = true;
				ALockInput = true;
				return true;
			}

			return false;
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class MitovGSMSerialSMSMessageFunction : public MitovGSMSerialSMSBasicMessageFunction
	{
		typedef MitovGSMSerialSMSBasicMessageFunction inherited;

	public:
		OpenWire::SourcePin	OutputPin;
		OpenWire::SourcePin	AddressOutputPin;
		OpenWire::SourcePin	NameOutputPin;
		OpenWire::SourcePin	TimeOutputPin;
		OpenWire::SourcePin	TimeZoneOutputPin;

	protected:
		bool FIsPDU = false;

	protected:
		bool ExtractTimeStamp( String ATimeStamp, TDateTime &ADateTime, int32_t &ATimeZone )
		{
			if( ATimeStamp.length() < 17 )
				return false;

			String AText = ATimeStamp.substring( 0, 2 );
			int16_t AYear = 2000 + AText.toInt();

			AText = ATimeStamp.substring( 3, 5 );
			int16_t AMonth = AText.toInt();

			AText = ATimeStamp.substring( 6, 8 );
			int16_t ADay = AText.toInt();

			AText = ATimeStamp.substring( 9, 11 );
			int16_t AHour = AText.toInt();

			AText = ATimeStamp.substring( 12, 14 );
			int16_t AMinute = AText.toInt();

			AText = ATimeStamp.substring( 15, 17 );
			int16_t ASecond = AText.toInt();

			AText = ATimeStamp.substring( 17, 20 );
			ATimeZone = AText.toInt();

			return ADateTime.TryEncodeDateTime( AYear, AMonth, ADay, AHour, AMinute, ASecond, 0 );
		}

	public:
		virtual void ProcessLine( String ALine, bool AIsSecondLine ) override
		{
			if( AIsSecondLine )
			{
				if( FIsPDU )
				{
//					Serial.println( ALine );
					// DODO: Decode!
					// http://soft.laogu.com/download/sms_pdu-mode.pdf
					// https://www.diafaan.com/sms-tutorials/gsm-modem-tutorial/online-sms-submit-pdu-decoder/
					// http://jazi.staff.ugm.ac.id/Mobile%20and%20Wireless%20Documents/s_gsm0705pdu.pdf
				}

				else
					OutputPin.SendValue( ALine );
			}

			else
			{
				FIsPDU = false;
				ALine.remove( 0, 5 );
				ALine.trim();
				String AAddressOrNameOrLength;
				if( Func::ExtractOptionallyQuotedCommaText( ALine, AAddressOrNameOrLength ))
				{
//					Serial.println( "TTT1" );
//					Serial.println( AAddressOrName );
					String ANameOrLength;
					if( Func::ExtractOptionallyQuotedCommaText( ALine, ANameOrLength ))
					{
//						Serial.println( "TTT2" );
						String ATimeStamp;
						if( Func::ExtractOptionallyQuotedCommaText( ALine, ATimeStamp ))
						{ 
							// Text Mode
							AddressOutputPin.SendValue( AAddressOrNameOrLength );
							NameOutputPin.SendValue( ANameOrLength );

							Mitov::TDateTime ADateTime;
							int32_t ATimeZone;
							if( ExtractTimeStamp( ATimeStamp, ADateTime, ATimeZone ))
							{
								TimeOutputPin.Notify( &ADateTime );
								TimeZoneOutputPin.Notify( &ATimeZone );
							}
						}

						else 
						{
//							Serial.println( "YYYYYYYYY" );
							FIsPDU = true;
//							int ALength = ANameOrLength.toInt();
							NameOutputPin.SendValue( AAddressOrNameOrLength );
						}
					}

					else
					{
//						Serial.println( "YYYYYYYYY" );
						FIsPDU = true;
	//					int ALength = ANameOrLength.toInt();
						NameOutputPin.Notify( (void *)"" );
					}
				}
			}
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class MitovArduinoGSMSerialDetectDefinedTextFunction : public MitovGSMSerialBasicFunction
	{
		typedef MitovGSMSerialBasicFunction inherited;

	public:
		OpenWire::SourcePin	OutputPin;

	protected:
		const char *FText;

	public:
		virtual bool TryProcessInput( String ALine, bool &ALockInput ) override
		{
			ALine.trim();
			if( ALine == FText ) //"RDY" )
			{
				OutputPin.Notify( nullptr );
				return true;
			}

			return false;
		}

	public:
		MitovArduinoGSMSerialDetectDefinedTextFunction(  MitovGSMSerial &AOwner, const char *AText ) : 
			inherited( AOwner ),
			FText( AText )
		{
		}

	};
//---------------------------------------------------------------------------
	class MitovGSMSerialSMSMessageReceivedFunctionRequestSMSDetails : public MitovGSMSerialBasicFunction
	{
		typedef MitovGSMSerialBasicFunction inherited;

	protected:
		MitovGSMSerialSMSMessageReceivedFunction &FOwnerFunction;

	protected:
		bool	FEmptyLineDetected = false;

	public:
		virtual bool TryProcessRequestedInput( String ALine, bool &ALockInput, bool &AResponseCompleted );

	public:
		MitovGSMSerialSMSMessageReceivedFunctionRequestSMSDetails( MitovGSMSerialSMSMessageReceivedFunction &AOwnerFunction );

	};
//---------------------------------------------------------------------------
	class MitovGSMSerialSMSMessageReceivedFunction : public MitovGSMSerialBasicFunction
	{
		typedef MitovGSMSerialBasicFunction inherited;

	public:
		OpenWire::SourcePin	StorageOutputPin;
		OpenWire::SourcePin	IndexOutputPin;
		OpenWire::SourcePin	ReceivedOutputPin;

	protected:
		MitovGSMSerialSMSMessageReceivedFunctionRequestSMSDetails	FRequestSMSDetails;

	protected:
		void RequestDetails( int32_t AIndex )
		{
			FOwner.SendQueryRegisterResponse( &FRequestSMSDetails, String( "AT+CMGR=" ) + AIndex + String( ",1" ));
		}

	public:
		virtual bool TryProcessInput( String ALine, bool &ALockInput ) override
		{
			ALine.trim();
			if( ALine.startsWith( "+CMTI:" ) )
			{
//				Serial.println( "ALine.startsWith" );
//				Serial.println( ALine );
				String AStorageType;
				if( Func::ExtractOptionallyQuotedCommaText( ALine, AStorageType ))
				{
					String AIndexText;
					if( Func::ExtractOptionallyQuotedCommaText( ALine, AIndexText ))
					{
						StorageOutputPin.SendValue( AStorageType );

						int32_t	AIndex = AIndexText.toInt();
						IndexOutputPin.Notify( &AIndex );

						RequestDetails( AIndex );

//						ReceivedOutputPin.Notify( nullptr );
					}
				}

				return true;
			}

			return false;
		}

		void ReportMessageDetails()
		{
			ReceivedOutputPin.Notify( nullptr );
		}

	public:
		MitovGSMSerialSMSMessageReceivedFunction( MitovGSMSerial &AOwner ) :
			inherited( AOwner ),
			FRequestSMSDetails( *this )
		{
		}

	};
//---------------------------------------------------------------------------
	class MitovSIM900ReadADCFunction : public MitovGSMSerialBasicFunction, public ClockingSupport
	{
		typedef MitovGSMSerialBasicFunction inherited;

	public:
		OpenWire::TypedSourcePin<float> OutputPin;
		OpenWire::TypedSourcePin<bool>	ErrorOutputPin;

	protected:
		bool FLocked : 1;
		bool FStarted : 1;
		bool FErrorStarted : 1;

	protected:
		bool	FEmptyLineDetected = false;

	protected:
		virtual void DoClockReceive( void *_Data ) override
		{
			FOwner.SendQueryRegisterResponse( this, "AT+CADC?" );
		}

	public:
		virtual bool TryProcessRequestedInput( String ALine, bool &ALockInput, bool &AResponseCompleted ) override
		{
			if( FLocked )
			{
				if( FEmptyLineDetected )
				{
					ALine.trim();
					if( ALine == "OK" )
					{
		//				Serial.println( "OK Processed" );
		//					Serial.println( "ALockInput = false" );
						AResponseCompleted = true;
						FEmptyLineDetected = false;
						ALockInput = false;
						return true;
					}
				}

				else if( ALine == "" )
				{
					FEmptyLineDetected = true;
					return true;
				}
			}

			if( ALine.startsWith( "+CADC:" ) )
			{
//				Serial.println( "ALine.startsWith" );
//				Serial.println( ALine );

				int APos = ALine.indexOf( ",", 6 );
				if( APos >= 0 )
				{
					String ALeft = ALine.substring( 6, APos );
					String ARight = ALine.substring( APos + 1 );
					ALeft.trim();
					ARight.trim();
					int ASuccess = ALeft.toInt();
					float AValue = ARight.toInt();

//					Serial.println( ASuccess );
//					Serial.println( AValue );

					ErrorOutputPin.SetValue( ASuccess != 0, FErrorStarted );
					FErrorStarted = true;

					if( ASuccess )
					{
						AValue /= 2800;
						OutputPin.SetValue( AValue, FStarted );
						FStarted = true;
					}
				}

				ALockInput = true;
				FLocked = true;
				return true;
			}

			return false;
		}

	public:
		MitovSIM900ReadADCFunction( MitovGSMSerial &AOwner ) :
			inherited( AOwner ),
			FLocked( false ),
			FStarted( false ),
			FErrorStarted( false )
		{
		}

	};
//---------------------------------------------------------------------------
	MitovGSMSerialBasicFunction::MitovGSMSerialBasicFunction( MitovGSMSerial &AOwner ) :
		FOwner( AOwner )
	{
//		Serial.println( "MitovGSMSerialBasicFunction" );
		FOwner.AddFunction( this );
	}
//---------------------------------------------------------------------------
	MitovGSMSerialReceivingVoiceCall::MitovGSMSerialReceivingVoiceCall( MitovGSMSerialVoiceCallFunction &AOwner ) :
		FOwner( AOwner )
	{
		AOwner.FReceivingCall = this;
		AnswerInputPin.SetCallback( &AOwner, (OpenWire::TOnPinReceive)&MitovGSMSerialVoiceCallFunction::DoReceiveAnswer );
	}
//---------------------------------------------------------------------------
	void MitovGSMSerialReceivingVoiceCall::Ringing()
	{
//		Serial.println( "RINGING!!!" );
		CallingOutputPin.Notify( nullptr );
		if( AutoAnswer.Enabled )
		{
			++FRingCount;
			if( FRingCount >= AutoAnswer.NumberRings )
				FOwner.DoReceiveAnswer( nullptr );
		}
	}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
	MitovGSMSerialBasicVoiceFunction::MitovGSMSerialBasicVoiceFunction( MitovGSMSerialVoiceCallFunction &AOwnerFunction ) :
		inherited( AOwnerFunction.FOwner ),
		FOwnerFunction( AOwnerFunction )
	{
	}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
	bool MitovGSMSerialVoiceCallAnswerFunction::TryProcessRequestedInput( String ALine, bool &ALockInput, bool &AResponseCompleted )
	{
		if( FEmptyLineDetected )
		{
			ALine.trim();
			if( ALine == "OK" )
			{
//				Serial.println( "OK Processed" );
//					Serial.println( "ALockInput = false" );
				AResponseCompleted = true;
				FEmptyLineDetected = false;
				ALockInput = false;
				return true;
			}
		}

		else if( ALine == "" )
		{
			FEmptyLineDetected = true;
			return true;
		}

		return false;
	}
//---------------------------------------------------------------------------
	MitovGSMSerialSMSMessageReceivedFunctionRequestSMSDetails::MitovGSMSerialSMSMessageReceivedFunctionRequestSMSDetails( MitovGSMSerialSMSMessageReceivedFunction &AOwnerFunction ) : 
		inherited( AOwnerFunction.FOwner ),
		FOwnerFunction( AOwnerFunction )
	{
	}
//---------------------------------------------------------------------------
	bool MitovGSMSerialSMSMessageReceivedFunctionRequestSMSDetails::TryProcessRequestedInput( String ALine, bool &ALockInput, bool &AResponseCompleted )
	{
		if( FEmptyLineDetected )
		{
			ALine.trim();
			if( ALine == "OK" )
			{
//				Serial.println( "OK Processed" );
//					Serial.println( "ALockInput = false" );
				AResponseCompleted = true;
				FEmptyLineDetected = false;
				ALockInput = false;
				return true;
			}
		}

		else if( ALine == "" )
		{
			FEmptyLineDetected = true;
			return true;
		}

		return false;
	}
//---------------------------------------------------------------------------
	void MitovGSMSerialPowerOnFunction::OKReceived()
	{
		FOwner.PowerOKReceived();
	}
//---------------------------------------------------------------------------
}

#endif
