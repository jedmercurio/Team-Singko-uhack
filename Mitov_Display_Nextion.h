////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_DISPLAY_NEXTION_h
#define _MITOV_DISPLAY_NEXTION_h

#include <Mitov.h>
#include <Mitov_Graphics.h>

namespace Mitov
{
	class ArduinoDisplayNextionElementBasic;
//---------------------------------------------------------------------------
	class DisplayNextionIntf
	{
	public:
		virtual void GetPosition( int32_t &AX, int32_t &AY ) { AX = 0; AY = 0; }
		virtual void SendCommand( const char *ACommand ) = 0;
		virtual void RegisterRender( ArduinoDisplayNextionElementBasic *AItem ) {}
		virtual	void ActiveatePage( ArduinoDisplayNextionElementBasic *APage ) = 0;
		virtual	bool IsPageActive( ArduinoDisplayNextionElementBasic *APage ) = 0;

	};
//---------------------------------------------------------------------------
	class ArduinoDisplayNextionElementBasic : public OpenWire::Object
	{
	protected:
		DisplayNextionIntf	&FOwner;

	public:
		virtual void Render() = 0;

	public:
		ArduinoDisplayNextionElementBasic( DisplayNextionIntf &AOwner ) :
			FOwner( AOwner )
		{
			FOwner.RegisterRender( this );
		}
	};
//---------------------------------------------------------------------------
	class DisplayNextionClockedElement : public ArduinoDisplayNextionElementBasic, public Mitov::ClockingSupport
	{
		typedef ArduinoDisplayNextionElementBasic inherited;

	public:
		using inherited::inherited;

	public:
		virtual void DoClockReceive( void *_Data ) override
		{
			Render();
		}

	};
//---------------------------------------------------------------------------
	class DisplayNextionElementFillScreen : public DisplayNextionClockedElement
	{
		typedef DisplayNextionClockedElement inherited;

	public:
		TColor Color;

	public:
		virtual void Render() override
		{
			FOwner.SendCommand( ( String( "cls " ) + Color.To565Color() ).c_str() );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class DisplayNextionElementFillRectangle : public DisplayNextionClockedElement
	{
		typedef DisplayNextionClockedElement inherited;

	public:
		TColor Color = TColor( 255, 0, 0 );
		int32_t		X = 0;
		int32_t		Y = 0;
		uint32_t	Width = 100;
		uint32_t	Height = 100;

	public:
		virtual void Render() override
		{
			int32_t AParentX, AParentY;
			FOwner.GetPosition( AParentX, AParentY );
			FOwner.SendCommand( ( String( "fill " ) + ( AParentX + X ) + "," + ( AParentY + Y ) + "," + Width + "," + Height + "," + Color.To565Color() ).c_str() );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class DisplayNextionElementDrawRectangle : public DisplayNextionClockedElement
	{
		typedef DisplayNextionClockedElement inherited;

	public:
		TColor Color = TColor( 255, 0, 0 );
		int32_t	X = 0;
		int32_t	Y = 0;
		uint32_t	Width = 100;
		uint32_t	Height = 100;

	public:
		virtual void Render() override
		{
			int32_t AParentX, AParentY;
			FOwner.GetPosition( AParentX, AParentY );
			FOwner.SendCommand( ( String( "draw " ) + ( AParentX + X ) + "," + ( AParentY + Y ) + "," + ( AParentX + X + Width ) + "," + ( AParentY + Y + Height ) + "," + Color.To565Color() ).c_str() );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------	
	class DisplayNextionElementDrawPicture : public DisplayNextionClockedElement
	{
		typedef DisplayNextionClockedElement inherited;

	public:
		int32_t	X = 0;
		int32_t	Y = 0;
		uint32_t	Width = 100;
		uint32_t	Height = 100;
		bool		Crop = true;
		uint32_t	PictureIndex = 0;

	public:
		virtual void Render() override
		{
			int32_t AParentX, AParentY;
			FOwner.GetPosition( AParentX, AParentY );
			if( Crop )
				FOwner.SendCommand( ( String( "picq " ) + ( AParentX + X ) + "," + ( AParentY + Y ) + "," + Width + "," + Height + "," + PictureIndex ).c_str() );

			else
				FOwner.SendCommand( ( String( "pic " ) + ( AParentX + X ) + "," + ( AParentY + Y ) + "," + PictureIndex ).c_str() );

		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class DisplayNextionElementActivatePage : public DisplayNextionClockedElement, public DisplayNextionIntf
	{
		typedef DisplayNextionClockedElement inherited;

	public:
		String	Page = "0";

	public:
		virtual	void ActiveatePage( ArduinoDisplayNextionElementBasic *APage ) override
		{
			FOwner.ActiveatePage( APage );
		}

		virtual	bool IsPageActive( ArduinoDisplayNextionElementBasic *APage ) override
		{
			return FOwner.IsPageActive( APage );
		}

		virtual void SendCommand( const char *ACommand ) override
		{
			if( FOwner.IsPageActive( this ))
				FOwner.SendCommand( ACommand );

		}

	public:
		virtual void Render() override
		{
			FOwner.ActiveatePage( this );
			int32_t AParentX, AParentY;
			FOwner.GetPosition( AParentX, AParentY );
			FOwner.SendCommand( ( String( "page " ) + Page ).c_str() );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class DisplayNextionAnalogGaugeValue
	{
	public:
		float	Value;
		int32_t	Angle;

	public:
		DisplayNextionAnalogGaugeValue( float AValue, int32_t AAngle ) :
			Value( AValue ),
			Angle( AAngle )
		{
		}
	};
//---------------------------------------------------------------------------
	class DisplayNextionElementAnalogGauge : public ArduinoDisplayNextionElementBasic
	{
		typedef ArduinoDisplayNextionElementBasic inherited;

	public:
		OpenWire::ValueChangeDetectSinkPin<float>	InputPin;

	public:
		String	ElementName = "z0";
		DisplayNextionAnalogGaugeValue	Min;
		DisplayNextionAnalogGaugeValue	Max;

	public:
		virtual void Render() override
		{
			int16_t AAngle = Func::MapRange( constrain( InputPin.Value, Min.Value, Max.Value ), Min.Value, Max.Value, float( Min.Angle ), float( Max.Angle ) ) + 0.5;
			if( AAngle < 0 )
				AAngle += 360;

			FOwner.SendCommand( ( ElementName + ".val=" + AAngle ).c_str() );
		}

	protected:
		void DoReceive( void *_Data )
		{
			Render();
		}

	public:
		DisplayNextionElementAnalogGauge( DisplayNextionIntf &AOwner ) :
			inherited( AOwner ),
			Min( 0.0, 0 ),
			Max( 1.0, 180 )
		{
			InputPin.SetCallback( this, (OpenWire::TOnPinReceive)&DisplayNextionElementAnalogGauge::DoReceive );
		}

	};
//---------------------------------------------------------------------------
	class DisplayNextionElementAnalogProgressBar : public ArduinoDisplayNextionElementBasic
	{
		typedef ArduinoDisplayNextionElementBasic inherited;

	public:
		OpenWire::ValueChangeDetectSinkPin<float>	InputPin;

	public:
		String	ElementName = "j0";
		float	Min = 0.0f;
		float	Max = 1.0f;

	public:
		virtual void Render() override
		{
			int16_t AAngle = Func::MapRange( constrain( InputPin.Value, Min, Max ), Min, Max, 0.0f, 100.0f ) + 0.5;
			FOwner.SendCommand( ( ElementName + ".val=" + AAngle ).c_str() );
		}

	protected:
		void DoReceive( void *_Data )
		{
			Render();
		}

	public:
		DisplayNextionElementAnalogProgressBar( DisplayNextionIntf &AOwner ) :
			inherited( AOwner )
		{
			InputPin.SetCallback( this, (OpenWire::TOnPinReceive)&DisplayNextionElementAnalogProgressBar::DoReceive );
		}

	};
//---------------------------------------------------------------------------
	class DisplayNextionElementText : public ArduinoDisplayNextionElementBasic
	{
		typedef ArduinoDisplayNextionElementBasic inherited;

	public:
		OpenWire::ValueChangeDetectSinkPin<String>	InputPin;

	public:
		String	ElementName = "t0";

	public:
		virtual void Render() override
		{
			FOwner.SendCommand( ( ElementName + ".txt=\"" + InputPin.Value + "\"" ).c_str() );
		}

	protected:
		void DoReceive( void *_Data )
		{
			Render();
		}

	public:
		DisplayNextionElementText( DisplayNextionIntf &AOwner ) :
			inherited( AOwner )
		{
			InputPin.SetCallback( this, (OpenWire::TOnPinReceive)&DisplayNextionElementText::DoReceive );
		}

	};
//---------------------------------------------------------------------------
	class DisplayNextionElementNumber : public ArduinoDisplayNextionElementBasic
	{
		typedef ArduinoDisplayNextionElementBasic inherited;

	public:
		OpenWire::ValueChangeDetectSinkPin<uint32_t>	InputPin;

	public:
		String	ElementName = "n0";

	public:
		virtual void Render() override
		{
			FOwner.SendCommand( ( ElementName + ".val=" + InputPin.Value ).c_str() );
		}

	protected:
		void DoReceive( void *_Data )
		{
			Render();
		}

	public:
		DisplayNextionElementNumber( DisplayNextionIntf &AOwner ) :
			inherited( AOwner )
		{
			InputPin.SetCallback( this, (OpenWire::TOnPinReceive)&DisplayNextionElementNumber::DoReceive );
		}

	};
//---------------------------------------------------------------------------
	class DisplayNextionElementFillCircle : public DisplayNextionClockedElement
	{
		typedef DisplayNextionClockedElement inherited;

	public:
		TColor Color = TColor( 255, 0, 0 );
		uint32_t	X = 50;
		uint32_t	Y = 50;
		uint32_t	Radius = 50;

	public:
		virtual void Render() override
		{
			int32_t AParentX, AParentY;
			FOwner.GetPosition( AParentX, AParentY );
			FOwner.SendCommand( ( String( "cirs " ) + ( AParentX + X ) + "," + ( AParentY + Y ) + "," + Radius + "," + Color.To565Color() ).c_str() );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class DisplayNextionElementDrawCircle : public DisplayNextionClockedElement
	{
		typedef DisplayNextionClockedElement inherited;

	public:
		TColor Color = TColor( 255, 0, 0 );
		uint32_t	X = 50;
		uint32_t	Y = 50;
		uint32_t	Radius = 50;

	public:
		virtual void Render() override
		{
			int32_t AParentX, AParentY;
			FOwner.GetPosition( AParentX, AParentY );
			FOwner.SendCommand( ( String( "cir " ) + ( AParentX + X ) + "," + ( AParentY + Y ) + "," + Radius + "," + Color.To565Color() ).c_str() );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class DisplayNextionElementDrawLine : public DisplayNextionClockedElement
	{
		typedef DisplayNextionClockedElement inherited;

	public:
		TColor Color = TColor( 255, 0, 0 );
		int32_t	X = 0;
		int32_t	Y = 0;
		int32_t	Width = 100;
		int32_t	Height = 100;

	public:
		virtual void Render() override
		{
			int32_t AParentX, AParentY;
			FOwner.GetPosition( AParentX, AParentY );
			FOwner.SendCommand( ( String( "line " ) + ( AParentX + X ) + "," + ( AParentY + Y ) + "," + ( AParentX + X + Width ) + "," + ( AParentY + Y + Height ) + "," + Color.To565Color() ).c_str() );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	enum TArduinoDisplayNextionTextBackgroundMode { bmClear, bmColor, bmPictureCrop, bmPicture };
//---------------------------------------------------------------------------
	class DisplayNextionTextBackground
	{
	public:
		TColor		Color;
		uint32_t	PictureIndex = 0;
		TArduinoDisplayNextionTextBackgroundMode Mode = bmClear;

	};
//---------------------------------------------------------------------------
	class DisplayNextionElementDrawText : public DisplayNextionClockedElement, OpenWire::Component
	{
		typedef DisplayNextionClockedElement inherited;

	public:
		TColor Color = TColor( 255, 0, 0 );
		DisplayNextionTextBackground	Background;
		int32_t	X = 0;
		int32_t	Y = 0;
		uint32_t	Width = 100;
		uint32_t	Height = 20;
		uint32_t	FontIndex = 0;
		TArduinoTextHorizontalAlign	HorizontalAlign : 2;
		TArduinoTextVerticalAlign	VerticalAlign : 2;
		String	InitialValue;

	protected:
		String	FValue;

	public:
		template<typename T> void Print( T AValue )
		{
			 FValue = String( AValue );
			 if( ! ClockInputPin.IsConnected() )
				 Render();
		}

	public:
		virtual void Render() override
		{
			int32_t AParentX, AParentY;
			FOwner.GetPosition( AParentX, AParentY );
			String ABackgroundColorText;
			String ABackgroundTypeText;
			switch( Background.Mode )
			{
				case bmClear:
					ABackgroundColorText = "NULL";
					ABackgroundTypeText = "1";
					break;

				case bmColor:
					ABackgroundColorText = Background.Color.To565Color();
					ABackgroundTypeText = "1";
					break;

				case bmPictureCrop:
					ABackgroundColorText = Background.PictureIndex;
					ABackgroundTypeText = "0";
					break;

				case bmPicture:
					ABackgroundColorText = Background.PictureIndex;
					ABackgroundTypeText = "2";
					break;

			}

			FOwner.SendCommand( ( String( "xstr " ) + ( AParentX + X ) + "," + ( AParentY + Y ) + "," + Width + "," + Height + "," + FontIndex + "," + Color.To565Color() + "," + ABackgroundColorText + "," + uint16_t( HorizontalAlign ) + "," + uint16_t( VerticalAlign ) + "," + ABackgroundTypeText + ",\"" + FValue + "\"" ).c_str() );
//			FOwner.SendCommand( ( String( "xstr " ) + ( AParentX + X ) + "," + ( AParentY + Y ) + "," + Width + "," + Height + "," + FontIndex + "," + Color.To565Color() + ",NULL," + uint16_t( HorizontalAlign ) + "," + uint16_t( VerticalAlign ) + ",1,\"" + FValue + "\"" ).c_str() );
		}

	protected:
		virtual void SystemInit() override
		{
			FValue = InitialValue;
//			inherited::SystemInit();
		}

	public:
		DisplayNextionElementDrawText( DisplayNextionIntf &AOwner ) :
			inherited( AOwner ),
			HorizontalAlign( thaCenter ),
			VerticalAlign( tvaCenter )
		{
		}

	};
//---------------------------------------------------------------------------
	class DisplayNextionElementDrawScene : public DisplayNextionClockedElement, public DisplayNextionIntf
	{
		typedef Mitov::DisplayNextionClockedElement inherited;

	public:
		uint32_t	X = 0;
		uint32_t	Y = 0;

	protected:
		Mitov::SimpleList<ArduinoDisplayNextionElementBasic *>	FElements;

	public:
		virtual	void ActiveatePage( ArduinoDisplayNextionElementBasic *APage ) override
		{
			FOwner.ActiveatePage( APage );
		}

		virtual	bool IsPageActive( ArduinoDisplayNextionElementBasic *APage ) override
		{
			return FOwner.IsPageActive( APage );
		}

		virtual void SendCommand( const char *ACommand ) override
		{
			FOwner.SendCommand( ACommand );
		}

	public:
		virtual void GetPosition( int32_t &AX, int32_t &AY )
		{ 
			int32_t AParentX, AParentY;
			FOwner.GetPosition( AParentX, AParentY );

			AX = AParentX + X;
			AY = AParentY + Y;
		}

		virtual void RegisterRender( ArduinoDisplayNextionElementBasic *AItem ) 
		{
			FElements.push_back( AItem );
		}

	public:
		virtual void Render() override
		{
			for( int i = 0; i < FElements.size(); ++ i )
				FElements[ i ]->Render();
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class DisplayNextion : public OpenWire::Component, public DisplayNextionIntf
	{
		typedef OpenWire::Component inherited;

	protected:
		ArduinoDisplayNextionElementBasic *FActivePage = nullptr;

	public:
		virtual	void ActiveatePage( ArduinoDisplayNextionElementBasic *APage ) override
		{
			FActivePage = APage;
		}

		virtual	bool IsPageActive( ArduinoDisplayNextionElementBasic *APage ) override
		{
			return ( APage == FActivePage );
		}

		virtual void SendCommand( const char *ACommand ) override
		{
//			Serial.println( ACommand );
			Stream &AStream = FSerial.GetStream();

			AStream.print( ACommand );
			AStream.print( "\xFF\xFF\xFF" );
		}

	protected:
		Mitov::BasicSerialPort &FSerial;

/*
		virtual void SystemLoopBegin( unsigned long currentMicros ) override
		{
//			Serial.println( "." );
//			if( FNeedsRead || ( ! ClockInputPin.IsConnected() ))
				ReadSensor();

			inherited::SystemLoopBegin( currentMicros );
		}
*/
	protected:
	
	public:
		DisplayNextion( Mitov::BasicSerialPort &ASerial ) :
			FSerial( ASerial )
		{
		}

	};
}

#endif
