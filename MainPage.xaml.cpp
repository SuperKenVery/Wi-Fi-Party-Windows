//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

#include "AudioRenderer.h"




using namespace Wi_Fi_Party_C___UWP;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

enum channel_type {
    music = 1,
    vocal = 2,
};

MainPage::MainPage()
{
    InitializeComponent();
    WSADATA wsaData;
    if (WSAStartup(0x0101, &wsaData)) {
        perror("WSAStartup");
        exit(1);
    }

    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        throw s;
    }

    u_int flag = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag)) < 0) {
        perror("Reuse addr");
        exit(1);
    }
    


    Receiver* r = new Receiver(s);
    Trackreceiver* music_receiver = new Trackreceiver();
    Trackreceiver* vocal_receiver = new Trackreceiver();
    Mixer<Trackreceiver>* sound_mixer = new Mixer<Trackreceiver>();
    sound_mixer->addSource(music_receiver);
    sound_mixer->addSource(vocal_receiver);
    Channel* music_channel = new Channel(music, channel_unreliable, 1, s, [=](Packet* p) {
        music_receiver->receiveSound(p);
        return true;
        });
    Channel* vocal_channel = new Channel(vocal, channel_unreliable, 1, s, [=](Packet* p) {
        vocal_receiver->receiveSound(p);
        return true;
        });
    r->addChannel(music_channel);
    r->addChannel(vocal_channel);

    char test_data[100] = { 0 };
    //vocal_channel->send(test_data, 100, 0);


    ComPtr<AudioRenderer> renderer = Make<AudioRenderer>(sound_mixer);

    r->receive_forever();

	//onExit: WSACleanup();
	
}


void Wi_Fi_Party_C___UWP::MainPage::Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
}


