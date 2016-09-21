//---------------------------------------------------------------------------
#include <stdio.h>
#include <vcl.h>
#include <registry.hpp>
#pragma hdrstop
#include <tlhelp32.h>

#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library

#include "Detect.h"
#include "s2aio_scratch2_arduino_killprocess.h"
#pragma package(smart_init)

bool EtatDetect = false;
bool EtatDetectAvant = true;
int NumDetectedCard = 0;

bool Answer_s2aio = false;
bool Answer_pymata = false;

SOCKET sock;
struct sockaddr_in server;
//---------------------------------------------------------------------------

//   Important : les méthodes et les propriétés des objets de la CVL peuvent uniquement
//   être utilisées dans une méthode appelée en utilisant Synchronize, comme suit :
//
//      Synchronize(&UpdateCaption);
//
//   où UpdateCaption serait de la forme :
//
//      void __fastcall TestCarte::UpdateCaption()
//      {
//        Form1->Caption = "Mis à jour dans un thread";
//      }
//---------------------------------------------------------------------------

__fastcall TestCarte::TestCarte(bool CreateSuspended)
	: TThread(CreateSuspended)
{

}
//---------------------------------------------------------------------------
bool TestCarte::isRunning(int pid)
 {   
   HANDLE pss = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);   
   
   PROCESSENTRY32 pe = { 0 };
   pe.dwSize = sizeof(pe);  
   
   if (Process32First(pss, &pe))   
   {  
	 while(Process32Next(pss, &pe))
     {  
       // pe.szExeFile can also be useful  
	   if (pe.th32ProcessID == pid)
         return true;   
	 }
   }
   
   CloseHandle(pss);  
    
   return false;  
 }
//---------------------------------------------------------------------------
void TestCarte::SetName()
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = "TestArduino";
	info.dwThreadID = -1;
	info.dwFlags = 0;

	__try
	{
		RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD),(DWORD*)&info );
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}
//---------------------------------------------------------------------------
void __fastcall TestCarte::Execute()
{
SetName();
FreeOnTerminate = true;
while (1)
   {
   if(Terminated) break;
   Synchronize(&Scan_Arduino);
   Sleep(250);
   }
}
//---------------------------------------------------------------------------
 void __fastcall TestCarte::Scan_Arduino()
{

if (isRunning(InterfaceS2A->PID_s2aio)||isRunning(InterfaceS2A->PID_pymata))
	{
	char *message, server_reply[2000];
	WSADATA wsa;
	WSAStartup(MAKEWORD(2,2),&wsa);

	if (InterfaceS2A->BlocklyArduino->Visible == true)
		{
		message = "GET /poll HTTP/1.1\r\n\r\n";
		sock = socket(AF_INET , SOCK_STREAM , 0 );
		//definition server s2aio on localhost:50209
		server.sin_addr.s_addr = inet_addr("127.0.0.1");
		server.sin_family = AF_INET;
		server.sin_port = htons( 50209 );
		}
   if (InterfaceS2A->RevenirsurScratch->Visible == true)
		{
		message = "GET /get_firmware_version HTTP/1.1\r\n\r\n";
		sock = socket(AF_UNSPEC , SOCK_STREAM , IPPROTO_TCP );
		//definition server PyMata on localhost:9000
		server.sin_addr.s_addr = inet_addr("127.0.0.1");
		server.sin_family = AF_INET;
		server.sin_port = htons( 9000 );
		}
	connect(sock , (struct sockaddr *)&server , sizeof(server));

	//envoi du poll pour vérifier que s2aio fonctionne
	send(sock , message , strlen(message) , 0);
	//réception de données
	if(recv(sock , server_reply , 2000 , 0) != SOCKET_ERROR) Answer_s2aio = true;
		else Answer_s2aio = false;
	} else Answer_s2aio = false;

if (isRunning(InterfaceS2A->PID_s2aio)||isRunning(InterfaceS2A->PID_pymata))
	{
	InterfaceS2A->Image_feu_rouge->Visible=true;
	InterfaceS2A->Image_feu_rouge->Enabled=true;
	InterfaceS2A->feu_rouge_vide->Visible=false;
	InterfaceS2A->feu_rouge_vide->Enabled=false;
	} else
		{
		InterfaceS2A->Image_feu_rouge->Visible=false;
		InterfaceS2A->Image_feu_rouge->Enabled=false;
		InterfaceS2A->feu_rouge_vide->Visible=true;
		InterfaceS2A->feu_rouge_vide->Enabled=true;
		}
if (isRunning(InterfaceS2A->PID_scratch) || isRunning(InterfaceS2A->PID_webserver))
	{
	InterfaceS2A->Image_feu_jaune->Visible=true;
	InterfaceS2A->Image_feu_jaune->Enabled=true;
	InterfaceS2A->feu_jaune_vide->Visible=false;
	InterfaceS2A->feu_jaune_vide->Enabled=false;

	if (Answer_s2aio)
		{
		InterfaceS2A->Image_feu_vert->Visible=true;
		InterfaceS2A->Image_feu_vert->Enabled=true;
		InterfaceS2A->feu_vert_vide->Visible=false;
		InterfaceS2A->feu_vert_vide->Enabled=false;
		}
	if (!Answer_s2aio)
		{
		InterfaceS2A->Image_feu_vert->Visible=false;
		InterfaceS2A->Image_feu_vert->Enabled=false;
		InterfaceS2A->feu_vert_vide->Visible=true;
		InterfaceS2A->feu_vert_vide->Enabled=true;
		}

	} else
		{
		InterfaceS2A->Image_feu_jaune->Visible=false;
		InterfaceS2A->Image_feu_jaune->Enabled=false;
		InterfaceS2A->feu_jaune_vide->Visible=true;
		InterfaceS2A->feu_jaune_vide->Enabled=true;
		InterfaceS2A->Image_feu_vert->Visible=false;
		InterfaceS2A->Image_feu_vert->Enabled=false;
		InterfaceS2A->feu_vert_vide->Visible=true;
		InterfaceS2A->feu_vert_vide->Enabled=true;
		}
EtatDetectAvant = EtatDetect;
//scan registry for Arduino
TRegistry *registre1 = new TRegistry();
TRegistry *registre2 = new TRegistry();  
TRegistry *registre3 = new TRegistry();
registre1->RootKey = HKEY_LOCAL_MACHINE;
registre2->RootKey = HKEY_LOCAL_MACHINE; 
registre3->RootKey = HKEY_LOCAL_MACHINE;
registre1->OpenKeyReadOnly("SYSTEM\\CurrentControlSet\\Services\\usbser\\Enum");
registre2->OpenKeyReadOnly("SYSTEM\\CurrentControlSet\\Services\\FTDIBUS\\Enum");  
registre3->OpenKeyReadOnly("SYSTEM\\CurrentControlSet\\Services\\CH341SER_A64\\Enum");
//valable seulement pour 1 carte Arduino, sinon incrémente...
//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\usbser\Enum -> 0 -> USB\VID_2341&PID_0042\6493633303735151D061
//HKEY_LOCAL_MACHINE\HARDWARE\DEVICEMAP\SERIALCOMM -> \Device\USBSER000 -> COMxx

//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\usbser\Enum -> 0 -> USB\VID_1A86
// pour ch340

//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\CH341SER_A64\Enum -> 0 -> USB\VID_1A86
// pour ch340 chinois


//Arduino n°1 ; FTDI n°2
if (registre1->ValueExists("0")||registre2->ValueExists("0")||registre3->ValueExists("0"))
	{
	EtatDetect = true;
	if (registre1->ValueExists("0")) NumDetectedCard = 1;
	if (registre2->ValueExists("0")) NumDetectedCard = 2;   
	if (registre3->ValueExists("0")) NumDetectedCard = 3;
	}
	else
		{
		EtatDetect = false;
		NumDetectedCard = 0;
		}
if (EtatDetectAvant != EtatDetect) {
	switch(NumDetectedCard)
	{
	case 0:
		{
		InterfaceS2A->ImgConnect->Picture->LoadFromFile("unconnect.bmp");
		InterfaceS2A->port=0;
		InterfaceS2A->Edit1->Clear();
		InterfaceS2A->Edit1->Text=IntToStr(InterfaceS2A->port);
		}
	case 1:
		{
		//valable pour cartes Arduino seules
		//test de la lecture de la valeur de la chaîne tronquée de 40 caractères
		//à partir du 13ème pour ne garder que le VID
		//ch340 port chinois, 2341 arduino, 2A03 leonardo, 0403 duemilanove
		//obligation de 2 tests pour minuscules compatibilité Windows XP
		if (((registre1->ReadString("0").Delete(13,40))=="USB\\VID_2341")||
			((registre1->ReadString("0").Delete(13,40))=="USB\\Vid_2341")||
			((registre1->ReadString("0").Delete(13,40))=="USB\\VID_2A03")||
			((registre1->ReadString("0").Delete(13,40))=="USB\\Vid_2A03")||
			((registre1->ReadString("0").Delete(13,40))=="USB\\VID_1A86")||
			((registre1->ReadString("0").Delete(13,40))=="USB\\Vid_1A86"))
			{
			TRegistry *registreCOM = new TRegistry();
			registreCOM->RootKey = HKEY_LOCAL_MACHINE;
			InterfaceS2A->ImgConnect->Picture->LoadFromFile("connect.bmp");
			registreCOM->OpenKeyReadOnly("HARDWARE\\DEVICEMAP\\SERIALCOMM");
			InterfaceS2A->port=StrToInt(registreCOM->ReadString("\\Device\\USBSER000").Delete(1,3));
			InterfaceS2A->Edit1->Clear();
			InterfaceS2A->Edit1->Text=IntToStr(InterfaceS2A->port);
			InterfaceS2A->INI->WriteInteger("port COM", "port", InterfaceS2A->port);
			delete registreCOM;
			}
		}
	case 2:
		{
		//valable pour cartes Arduino avec puce FTDI   VID = 0403 , PID = 6001, 6010 ou 6011
		//test de la lecture de la valeur de la chaîne tronquée de 40 caractères
		//à partir du 13ème pour ne garder que le VID
		//obligation de 2 tests pour minuscules compatibilité Windows XP
		if (((registre2->ReadString("0").Delete(13,41))=="USB\\VID_0403")||
			((registre2->ReadString("0").Delete(13,41))=="USB\\Vid_0403"))
			{
			TRegistry *registreCOM = new TRegistry();
			registreCOM->RootKey = HKEY_LOCAL_MACHINE;
			InterfaceS2A->ImgConnect->Picture->LoadFromFile("connect.bmp");
			registreCOM->OpenKeyReadOnly("HARDWARE\\DEVICEMAP\\SERIALCOMM");
			InterfaceS2A->port=StrToInt(registreCOM->ReadString("\\Device\\VCP0").Delete(1,3));    
			InterfaceS2A->Edit1->Clear();
			InterfaceS2A->Edit1->Text=IntToStr(InterfaceS2A->port);
			InterfaceS2A->INI->WriteInteger("port COM", "port", InterfaceS2A->port);
			delete registreCOM;
			}
		}      
	case 3:
		{
		//valable pour cartes Arduino seules
		//test de la lecture de la valeur de la chaîne tronquée de 40 caractères
		//à partir du 13ème pour ne garder que le VID
		//ch340 port chinois, 2341 arduino, 2A03 leonardo, 0403 duemilanove 
		//obligation de 2 tests pour minuscules compatibilité Windows XP
		if (((registre3->ReadString("0").Delete(13,46))=="USB\\VID_1A86")||
			((registre3->ReadString("0").Delete(13,46))=="USB\\Vid_1A86"))
			{
			TRegistry *registreCOM = new TRegistry();
			registreCOM->RootKey = HKEY_LOCAL_MACHINE;
			InterfaceS2A->ImgConnect->Picture->LoadFromFile("connect.bmp");
			registreCOM->OpenKeyReadOnly("HARDWARE\\DEVICEMAP\\SERIALCOMM");
			InterfaceS2A->port=StrToInt(registreCOM->ReadString("\\Device\\Serial2").Delete(1,3));     
			InterfaceS2A->Edit1->Clear();
			InterfaceS2A->Edit1->Text=IntToStr(InterfaceS2A->port);
			InterfaceS2A->INI->WriteInteger("port COM", "port", InterfaceS2A->port);
			delete registreCOM;
			}
		}
	}
	}
delete registre1;
delete registre2;
delete registre3;
}
