//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

#include <Audioclient.h>
#include <Audiopolicy.h>
#include <WinSock2.h>
#include <Windows.h>
#include <wrl\implements.h>
#include <mfapi.h>
#include <mmdeviceapi.h>

using namespace Microsoft::WRL;
using namespace Windows::Media::Devices;
using namespace Windows::Storage::Streams;




namespace Wi_Fi_Party_C___UWP
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

	private:
		void Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};

	
    
}
