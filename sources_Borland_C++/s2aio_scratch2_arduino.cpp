﻿//---------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN
#include <vcl.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>

#pragma hdrstop

#include <Windows.h>
#include <setupapi.h>

using namespace std;  //pour les traductions
#include <tinyxml.h>

#include <IniFiles.hpp> //pour le fichier INI
#include <registry.hpp>

#include "s2aio_scratch2_arduino.h"
#include "Detect.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

TInterfaceS2A *InterfaceS2A;

//---------------------------------------------------------------------------

TIniFile *INI = new TIniFile(ExtractFilePath(Application->ExeName)+ "s2aio.ini");
//listes pour récupérer le contenu du fichier INI
int port=0;
int choix_langue;
AnsiString locate_scratch2="Scratch 2.exe";
AnsiString locate_librairie;
AnsiString locate_project;
AnsiString locate_doc;
AnsiString arg="";
//listes pour les menus dynamiques
TStringList *Examples = new TStringList;
TStringList *Biblio = new TStringList;
TStringList *Aides = new TStringList;
TStringList *Docs = new TStringList;
TStringList *Extensions1 = new TStringList;
TStringList *Extensions2 = new TStringList;

bool BlocklyOUScratch = false;

//---------------------------------------------------------------------------
void __fastcall TInterfaceS2A::InitINI()
{
  //écriture dans le fichier INI des variables utiles
  INI->WriteInteger("port COM", "port", 0);      //port par défaut pour initialiser
  INI->WriteString("locate Scratch2", "locate_scratch2", "Scratch 2.exe"); //chemin par défaut pour initialiser
  INI->WriteInteger("langue", "langue", 0); //dans le fichier label.xml, à la balise <Langues>, cela correspond au n° du rang de la langue
  //dossiers à utiliser pour les documents
  INI->WriteString("locate Librairie", "locate_librairie", ExtractFilePath(Application->ExeName) + "bibliotheque\\"); //chemin par défaut pour initialiser
  INI->WriteString("locate Project", "locate_project", ExtractFilePath(Application->ExeName) + "projets\\"); //chemin par défaut pour initialiser
  INI->WriteString("locate Documentation", "locate_doc", ExtractFilePath(Application->ExeName) + "documentation\\"); //chemin par défaut pour initialiser
}
//---------------------------------------------------------------------------
__fastcall TInterfaceS2A::TInterfaceS2A(TComponent* Owner)
	: TForm(Owner)
{
  InterfaceS2A->ClientWidth=530;
  InterfaceS2A->Constraints->MinWidth=530;
  InterfaceS2A->Constraints->MaxWidth=530;
  InterfaceS2A->Height=240;

  this->ImgConnect->Picture->LoadFromFile("unconnect.bmp");
  //vérification de l'existence du fichier INI, sinon le recréé
  if (!FileExists(ExtractFilePath(Application->ExeName)+ "s2aio.ini"))
	 InitINI();
  //lecture du fichier INI
  port=INI->ReadInteger("port COM", "port", 0);
  choix_langue=INI->ReadInteger("langue", "langue", 0);
  locate_scratch2=INI->ReadString("locate Scratch2", "locate_scratch2", "Scratch 2.exe");
  locate_librairie=INI->ReadString("locate Librairie", "locate_librairie", ExtractFilePath(Application->ExeName) + "bibliotheque\\");
  locate_project=INI->ReadString("locate Project", "locate_project", ExtractFilePath(Application->ExeName) + "projets\\");
  locate_doc=INI->ReadString("locate Documentation", "locate_doc", ExtractFilePath(Application->ExeName) + "documentation\\");

  //nettoie le champ du TEdit et y place la valeur lue dans le INI
  Edit1->Clear();
  //procédure de contrôle si le fichier INI est mal rempli
  port=INI->ReadInteger("port COM", "port", 0);
  Edit1->Text=IntToStr(port);

  Extensions1->Add(".sb2");
  // Appel à Search, avec pour liste des fichiers, les items des listes Bibilo & Examples.
  SearchEx(locate_librairie, Extensions1, Biblio,0);
  SearchEx(locate_project, Extensions1, Examples,1);
  delete Extensions1;
  Extensions2->Add(".pdf");
  Extensions2->Add(".odt");
  Extensions2->Add(".lnk");
  Extensions2->Add(".url");
  Extensions2->Add(".html");
  Extensions2->Add(".htm");  
  // Appel à SearchDocs, avec pour liste des fichiers les items de Documentation.
  SearchDocs(locate_doc, Extensions2, Docs,2);
  delete Extensions2;

  //appel à la procédure pour gérer les langues
  AnsiString file = ExtractFilePath(Application->ExeName) + "label.xml";
  langue = new GestionLangue;
  langue->Init(InterfaceS2A->Langue1,file.c_str(),(ptrOnClick)&Langue1Click);
  //après l'initialisation des langues, le système pioche la langue précédemment sélectionnée
  langue->Change(choix_langue);
  TestCarte *ThreadTestArduino=new TestCarte(false);
  Edit1->Text="1";

  //vérification d'argument sur la ligne de commande
  arg=GetCommandLine();
  arg=arg.Delete(1,7);  //élimine les 5 premiers caractères : s2aio_
  if(arg=="-i") InitINI();
  Environnement(BlocklyOUScratch);
}
//-------------------------recherche des fichiers sb2 pour les lister dans le menu Fichier--------------------------------------------------
void __fastcall TInterfaceS2A::Environnement(bool BouS)
{
//true if Blockly fisrt, else false if Scratch
if (BouS == true)
	{
	  InterfaceS2A->BlocklyArduino->Visible=false;
	  InterfaceS2A->RevenirsurScratch->Visible=true;
	  InterfaceS2A->Button1->Enabled=false;
	  InterfaceS2A->Button2->Enabled=false; 
	  InterfaceS2A->Button1->Visible=false;
	  InterfaceS2A->Button2->Visible=false;
	  InterfaceS2A->Button3->Enabled=true;
	  InterfaceS2A->Button4->Enabled=true;
	  InterfaceS2A->Button3->Visible=true;
	  InterfaceS2A->Button4->Visible=true;
	  InterfaceS2A->Image1->Visible=false;
	  InterfaceS2A->Image3->Visible=true;
	  /*InterfaceS2A->Image_feu_vert->Left=351;
	  InterfaceS2A->Image_feu_vert->Top=265;
	  InterfaceS2A->Image_feu_jaune->Left=351;
	  InterfaceS2A->Image_feu_jaune->Top=214;
	  InterfaceS2A->Image_feu_rouge->Left=24;
	  InterfaceS2A->Image_feu_rouge->Top=265;
	  InterfaceS2A->Image_feu_vide->Left=24;
	  InterfaceS2A->Image_feu_vide->Top=214;*/
	}
	else if (BouS == false)
	{
	  InterfaceS2A->BlocklyArduino->Visible=true;
	  InterfaceS2A->RevenirsurScratch->Visible=false;
	  InterfaceS2A->Button1->Enabled=true;
	  InterfaceS2A->Button2->Enabled=true;  
	  InterfaceS2A->Button1->Visible=true;
	  InterfaceS2A->Button2->Visible=true;
	  InterfaceS2A->Button3->Enabled=false;
	  InterfaceS2A->Button4->Enabled=false;
	  InterfaceS2A->Button3->Visible=false;
	  InterfaceS2A->Button4->Visible=false;
	  InterfaceS2A->Image1->Visible=true;
	  InterfaceS2A->Image3->Visible=false;
	  /*InterfaceS2A->Image_feu_vert->Left=351;
	  InterfaceS2A->Image_feu_vert->Top=265;
	  InterfaceS2A->Image_feu_jaune->Left=351;
	  InterfaceS2A->Image_feu_jaune->Top=214;
	  InterfaceS2A->Image_feu_rouge->Left=24;
	  InterfaceS2A->Image_feu_rouge->Top=265;
	  //Scratch + feux vides
	  InterfaceS2A->Image_feu_vide->Left=360;
	  InterfaceS2A->Image_feu_vide->Top=150;*/
	}
}
//-------------------------recherche des fichiers sb2 pour les lister dans le menu Fichier--------------------------------------------------
void __fastcall TInterfaceS2A::SearchEx(AnsiString FilePath, TStringList * Extensions, TStrings * ListeFichiers, int RangMenu)
{
  TSearchRec Infos_fichier;
  TMenuItem *NewItem = new TMenuItem(this);
  if (!FindFirst(FilePath+"*.*", faAnyFile, Infos_fichier))
  do
  {
	for (int i = 0 ; i < Extensions->Count ; i++)
	{
	  if (ExtractFileExt(Infos_fichier.Name).LowerCase() == Extensions->Strings[i].LowerCase())
	  {
		//ajout dans la liste d'une ligne avec le chemin+nom du fichier trouvé
		ListeFichiers->Add(ExpandFileName(Infos_fichier.Name));
		//nettoie le nom de fichier de son extension sb2 pour le nom seul du fichier, sans le chemin
		AnsiString CleanName = StringReplace(Infos_fichier.Name, ".sb2", "",TReplaceFlags() << rfReplaceAll);
		//rajout d'entrées dans le menu
		NewItem = new TMenuItem(this);
		NewItem->Caption = CleanName;
		NewItem->Name = CleanName;
		//stockage du chemin complet + nom de fichier pour la fonction ExempleClick
		NewItem->Hint = FilePath + Infos_fichier.Name.c_str();
		NewItem->OnClick=ExempleClick;
		//rajout au bon endroit de l'entrée dans le menu
		MainMenu1->Items->Items[0]->Items[RangMenu]->Add(NewItem);
	  }
	}
  }
  while(!FindNext(Infos_fichier));
  FindClose(Infos_fichier);
}
//---------------------associer chaque entrée dynamique dans Biblio & Exemples à leur lancement-------------------
void __fastcall TInterfaceS2A::ExempleClick(TObject *Sender)
{
AnsiString CheminNomFichier;
ofstream fichier_s2("scratch2.bat", ios::out | ios::trunc);  // ouverture en écriture avec effacement du fichier ouvert
		if(fichier_s2)
		{
			   //récupération des infos de l'entrée TMenu qui a déclenché cette fonction
			   CheminNomFichier=static_cast<TMenuItem*>(Sender)->Hint;
			   fichier_s2 << "@echo off\nbreak ON\nrem fichiers BAT et fork créés par Sébastien CANET\ncls\nSET currentpath=%~dp1\nSET dossier_scratch="<< '"' << locate_scratch2.c_str() << '"'<< "\nstart %dossier_scratch% " << '"' << CheminNomFichier.c_str() << '"';
			   fichier_s2.close();
		}
		else ShowMessage(Popup->Items->Strings[0]);
ShellExecute(0, 0, "scratch2.bat", 0, 0 , SW_HIDE );
}
//-------------------------recherche des fichiers de docs pour les lister dans les menus Aide & Documentation---------------
void __fastcall TInterfaceS2A::SearchDocs(AnsiString FilePath, TStringList * Extensions, TStrings * ListeFichiers, int RangMenu)
{
  TSearchRec Infos_fichier;
  TMenuItem *NewItem = new TMenuItem(this);
  if (!FindFirst(FilePath+"*.*", faAnyFile, Infos_fichier))
  do
  {
	for (int i = 0 ; i < Extensions->Count ; i++)
	{
	  if (ExtractFileExt(Infos_fichier.Name).LowerCase() == Extensions->Strings[i].LowerCase())
	  {
		//ajout dans la liste d'une ligne avec le chemin+nom du fichier trouvé
		ListeFichiers->Add(ExpandFileName(Infos_fichier.Name));
		//nettoie le nom de fichier de son extension pour le nom seul du fichier, sans le chemin
		AnsiString CleanName = StringReplace(Infos_fichier.Name, Extensions->Strings[i].LowerCase(), "",TReplaceFlags() << rfReplaceAll);
		//rajout d'entrées dans le menu
		NewItem = new TMenuItem(this);
		NewItem->Caption = Infos_fichier.Name;
		NewItem->Name = CleanName;
		//stockage du chemin complet + nom de fichier pour la fonction ExempleClick
		NewItem->Hint = FilePath + Infos_fichier.Name.c_str();
		NewItem->OnClick=DocsClick;
		//rajout au bon endroit de l'entrée dans le menu
		MainMenu1->Items->Items[RangMenu]->Add(NewItem);
	  }
	}
  }
  while(!FindNext(Infos_fichier));
  FindClose(Infos_fichier);
}
//---------------------associer chaque entrée dynamique dans Biblio & Exemples à leur lancement-------------------
void __fastcall TInterfaceS2A::DocsClick(TObject *Sender)
{
//récupération des infos de l'entrée TMenu qui a déclenché cette fonction
AnsiString CheminNomFichier=static_cast<TMenuItem*>(Sender)->Hint;

ShellExecute(0, 0, CheminNomFichier.c_str(), 0, 0 , SW_HIDE );
}
//---------------------------------------------------------------------------
void __fastcall TInterfaceS2A::A_proposClick(TObject *Sender)
{
ShellExecute(0, 0, "https://github.com/technologiescollege/s2aio-control-panel", 0, 0 , SW_SHOW );
}
//---------------------------------------------------------------------------
void __fastcall TInterfaceS2A::Siteofficieldes2a1Click(TObject *Sender)
{
ShellExecute(0, 0, "http://mryslab.blogspot.fr/", 0, 0 , SW_SHOW );
}
//---------------------------------------------------------------------------
void __fastcall TInterfaceS2A::PrparerArduino1Click(TObject *Sender)
{
ShellExecute(0, 0, ".\\aide\\1.pdf", 0, 0 , SW_SHOW );
}
//---------------------------------------------------------------------------
void __fastcall TInterfaceS2A::Utilisation1Click(TObject *Sender)
{
ShellExecute(0, 0, ".\\aide\\2.pdf", 0, 0 , SW_SHOW );
}
//---------------------------------------------------------------------------
void __fastcall TInterfaceS2A::N4Principesdeprogrammation1Click(TObject *Sender)
{
ShellExecute(0, 0, ".\\aide\\3.pdf", 0, 0 , SW_SHOW );
}
//---------------------------------------------------------------------------
void __fastcall TInterfaceS2A::N3Mmentodescommandes1Click(TObject *Sender)
{
ShellExecute(0, 0, ".\\aide\\4.pdf", 0, 0 , SW_SHOW );
}
//---------------------------------------------------------------------------
void __fastcall TInterfaceS2A::Wiki1Click(TObject *Sender)
{
ShellExecute(0, 0, "http://tic.technologiescollege.fr/wiki", 0, 0 , SW_SHOW );
}
//---------------------------------------------------------------------------
void __fastcall TInterfaceS2A::Button2Click(TObject *Sender)
{
ofstream fichier_s2("scratch2.bat", ios::out | ios::trunc);  // ouverture en écriture avec effacement du fichier ouvert
		if(fichier_s2)
		{
				fichier_s2 << "@echo off\nbreak ON\nrem fichiers BAT et fork créés par Sébastien CANET\ncls\nSET currentpath=%~dp1\nSET dossier_scratch="<< '"' << locate_scratch2.c_str() << '"'<< "\nstart %dossier_scratch% " << '"' << locate_librairie.c_str() << "s2aio_Scratch2Arduino.sb2" << '"' ;
				fichier_s2.close();
		}
		else ShowMessage(Popup->Items->Strings[1]);
ShellExecute(0, 0, "scratch2.bat", 0, 0 , SW_HIDE );
}
//---------------------------------------------------------------------------
void __fastcall TInterfaceS2A::Button1Click(TObject *Sender)
{
port=this->Edit1->Text.ToInt();
INI->WriteInteger("port COM", "port", port);
ofstream fichier_s2a("s2aio_cmd.bat", ios::out | ios::trunc);  // ouverture en écriture avec effacement du fichier ouvert
		if(fichier_s2a)
		{
				fichier_s2a << "@echo off\nbreak ON\nrem fichiers BAT et fork créés par Sébastien CANET\nSET currentpath=%~dp1\ncd %currentpath%s2aio\ncls\npython.exe .\\Lib\\site-packages\\s2aio\\__main__.py -l 5 -c no_client -p COM" << port;
				fichier_s2a.close();
		}
		else ShowMessage(Popup->Items->Strings[2]);
ShellExecute(0, 0, "s2aio_cmd.bat", 0, 0 , SW_SHOW );
}
//---------------------------------------------------------------------------
void __fastcall TInterfaceS2A::Quitter1Click(TObject *Sender)
{
InterfaceS2A->Close();
}
//---------------------------------------------------------------------------
void __fastcall TInterfaceS2A::OuvrirClick(TObject *Sender)
{
if (Open_s2->Execute()==true) locate_scratch2=Open_s2->FileName.c_str();
//écriture dans le fichier INI
//DeleteKey("locate Scratch2", "locate_scratch2");
INI->WriteString("locate Scratch2", "locate_scratch2", locate_scratch2);
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::Edit1KeyPress(TObject *Sender, char &Key)
{
if (Key < '0' || Key >'9')
	{
	ShowMessage(Popup->Items->Strings[3]);
	Key=NULL;
	}
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::Apropos1Click(TObject *Sender)
{
ShowMessage(Popup->Items->Strings[4]);
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::Gestionnairedepriphriques1Click(TObject *Sender)
{
ShellExecute(0, 0, "devmgmt.msc", 0, 0 , SW_HIDE );
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::Langue1Click(TObject *Sender)
{
INI->WriteInteger("langue", "langue", ((TMenuItem*)Sender)->Tag);
langue->Change(((TMenuItem*)Sender)->Tag);
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::InitClick(TObject *Sender)
{
DeleteFile("s2aio.ini");
InitINI();
//nettoie le champ du TEdit et y place la valeur lue dans le INI
Edit1->Clear();
//procédure de contrôle si le fichier INI est mal rempli
port=INI->ReadInteger("port COM", "port", 3);
Edit1->Text=IntToStr(port);
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::SiteofficielArduino1Click(TObject *Sender)
{
ShellExecute(0, 0, "http://arduino.cc/en/Main/Software", 0, 0 , SW_SHOW );
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::lchargerlesdrivers1Click(TObject *Sender)
{
ShellExecute(0, 0, "https://github.com/technologiescollege/ressources/tree/master/drivers", 0, 0 , SW_SHOW );
}

//---------------------------------------------------------------------------
void __fastcall TInterfaceS2A::DlScratch2Click(TObject *Sender)
{
ShellExecute(0, 0, "https://scratch.mit.edu/scratch2download/", 0, 0 , SW_SHOW );
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::Localiserledossierexemples1Click(TObject *Sender)
{
if (OpenDialog->Execute()==true) locate_librairie=ExtractFilePath(OpenDialog->FileName);
//écriture dans le fichier INI
INI->WriteString("locate Librairie", "locate_librairie", locate_librairie);
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::Localiserledossierprojets1Click(TObject *Sender)
{
if (OpenDialog->Execute()==true) locate_project=ExtractFilePath(OpenDialog->FileName);
//écriture dans le fichier INI
INI->WriteString("locate Project", "locate_project", locate_project);
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::Localiserledossierdocumentation1Click(
      TObject *Sender)
{
if (OpenDialog->Execute()==true) locate_doc=ExtractFilePath(OpenDialog->FileName);
//écriture dans le fichier INI
INI->WriteString("locate Documentation", "locate_doc", locate_doc);
}
//---------------------------------------------------------------------------
void __fastcall TInterfaceS2A::Firmata_UnoClick(TObject *Sender)
{
port=this->Edit1->Text.ToInt();
INI->WriteInteger("port COM", "port", port);
ofstream fichier_s2(".\\tools\\flash_uno.bat", ios::out | ios::trunc);  // ouverture en écriture avec effacement du fichier ouvert
		if(fichier_s2)
		{
				fichier_s2 << "@echo off\nbreak ON\nrem fichiers BAT et fork créés par Sébastien CANET\nSET currentpath=%~dp1\ncd %currentpath%tools\ncls\n.\\avrdude -Cavrdude.conf -v -patmega328p -carduino -P\\" << "\\" << ".\\COM" << port << " -b115200 -D -V -Uflash:w:s2aio-FirmataPlus.Uno.hex:i" << "\npause";
				fichier_s2.close();
		}
		else ShowMessage(Popup->Items->Strings[1]);
ShellExecute(0, 0, ".\\tools\\flash_uno.bat", 0, 0 , SW_SHOW );
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::Firmata_MegaClick(TObject *Sender)
{
port=this->Edit1->Text.ToInt();
INI->WriteInteger("port COM", "port", port);
ofstream fichier_s2(".\\tools\\flash_mega.bat", ios::out | ios::trunc);  // ouverture en écriture avec effacement du fichier ouvert
		if(fichier_s2)
		{
				fichier_s2 << "@echo off\nbreak ON\nrem fichiers BAT et fork créés par Sébastien CANET\nSET currentpath=%~dp1\ncd %currentpath%tools\ncls\n.\\avrdude -Cavrdude.conf -v -patmega2560 -cwiring -P\\" << "\\" << ".\\COM" << port << " -b115200 -D -V -Uflash:w:s2aio-FirmataPlus.Mega.hex:i" << "\npause";
				fichier_s2.close();
		}
		else ShowMessage(Popup->Items->Strings[1]);
ShellExecute(0, 0, ".\\tools\\flash_mega.bat", 0, 0 , SW_SHOW );
}

//---------------------------------------------------------------------------


void __fastcall TInterfaceS2A::Firmata_LeonardoClick(TObject *Sender)
{
port=this->Edit1->Text.ToInt();
INI->WriteInteger("port COM", "port", port);
ofstream fichier_s2(".\\tools\\flash_leonardo.bat", ios::out | ios::trunc);  // ouverture en écriture avec effacement du fichier ouvert
		if(fichier_s2)
		{
				fichier_s2 << "@echo off\nbreak ON\nrem fichiers BAT et fork créés par Sébastien CANET\nSET currentpath=%~dp1\ncd %currentpath%tools\ncls\n.\\avrdude -Cavrdude.conf -v -patmega32u4 -cavr109 -P\\" << "\\" << ".\\COM" << port << " -b57600 -D -V -Uflash:w:s2aio-FirmataPlus.Leonardo.hex:i" << "\npause";
				fichier_s2.close();
		}
		else ShowMessage(Popup->Items->Strings[1]);
ShellExecute(0, 0, ".\\tools\\flash_leonardo.bat", 0, 0 , SW_SHOW );
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::Firmata_MicroClick(TObject *Sender)
{
port=this->Edit1->Text.ToInt();
INI->WriteInteger("port COM", "port", port);
ofstream fichier_s2(".\\tools\\flash_micro.bat", ios::out | ios::trunc);  // ouverture en écriture avec effacement du fichier ouvert
		if(fichier_s2)
		{
				fichier_s2 << "@echo off\nbreak ON\nrem fichiers BAT et fork créés par Sébastien CANET\nSET currentpath=%~dp1\ncd %currentpath%tools\ncls\n.\\avrdude -Cavrdude.conf -v -patmega32u4 -cavr109 -P\\" << "\\" << ".\\COM" << port << " -b57600 -D -V -Uflash:w:s2aio-FirmataPlus.Micro.hex:i" << "\npause";
				fichier_s2.close();
		}
		else ShowMessage(Popup->Items->Strings[1]);
ShellExecute(0, 0, ".\\tools\\flash_micro.bat", 0, 0 , SW_SHOW );
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::Firmata_NanoClick(TObject *Sender)
{
port=this->Edit1->Text.ToInt();
INI->WriteInteger("port COM", "port", port);
ofstream fichier_s2(".\\tools\\flash_nano.bat", ios::out | ios::trunc);  // ouverture en écriture avec effacement du fichier ouvert
		if(fichier_s2)
		{
				fichier_s2 << "@echo off\nbreak ON\nrem fichiers BAT et fork créés par Sébastien CANET\nSET currentpath=%~dp1\ncd %currentpath%tools\ncls\n.\\avrdude -Cavrdude.conf -v -patmega328p -carduino -P\\" << "\\" << ".\\COM" << port << " -b57600 -D -V -Uflash:w:s2aio-FirmataPlus.Nano.hex:i" << "\npause";
				fichier_s2.close();
		}
		else ShowMessage(Popup->Items->Strings[1]);
ShellExecute(0, 0, ".\\tools\\flash_nano.bat", 0, 0 , SW_SHOW );
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::w32Click(TObject *Sender)
{
ShellExecute(0, 0, "dpinst-x86.exe", 0, (ExtractFilePath(Application->ExeName) + "drivers").c_str() , SW_SHOW );
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::w64Click(TObject *Sender)
{
ShellExecute(0, 0, "dpinst-amd64.exe", 0, (ExtractFilePath(Application->ExeName) + "drivers").c_str() , SW_SHOW );
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::DriversClick(TObject *Sender)
{
ShellExecute(0, 0, "explorer.exe", (ExtractFilePath(Application->ExeName) + "drivers").c_str(), 0, SW_SHOW );
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::Edit1Change(TObject *Sender)     //évènement récupéré du thread de détection de carte
{

if (Edit1->Text=="0") ShowMessage(Popup->Items->Strings[5] + "\n" + Popup->Items->Strings[6] + "\n" + Popup->Items->Strings[7]);
/*
if (Edit1->Text == "1")
	{
	TRegistry *registre1 = new TRegistry();
	TRegistry *registre2 = new TRegistry();
	registre1->RootKey = HKEY_LOCAL_MACHINE;
	registre2->RootKey = HKEY_LOCAL_MACHINE;
	if (registre1->OpenKeyReadOnly("SYSTEM\\CurrentControlSet\\Services\\usbser\\Enum"))
		{
        //test de la lecture de la valeur de la chaîne tronquée de 40 caractères
		//à partir du 13ème pour ne garder que le VID
		//obligation de 2 tests pour minuscules compatibilité Windows XP
		if (((registre1->ReadString("0").Delete(13,40))=="USB\\VID_2341")||
			((registre1->ReadString("0").Delete(13,40))=="USB\\Vid_2341")||
			((registre1->ReadString("0").Delete(13,40))=="USB\\VID_2A03")||
			((registre1->ReadString("0").Delete(13,40))=="USB\\Vid_2A03"))
			{
			this->ImgConnect->Picture->LoadFromFile("connect.bmp");
			registre2->OpenKeyReadOnly("HARDWARE\\DEVICEMAP\\SERIALCOMM");
			port=StrToInt(registre2->ReadString("\\Device\\USBSER000").Delete(1,3));
			Edit1->Clear();
			Edit1->Text=IntToStr(port);
			INI->WriteInteger("port COM", "port", port);
			}
		else
			{
			this->ImgConnect->Picture->LoadFromFile("unconnect.bmp");
			port=0;
			Edit1->Clear();
			Edit1->Text=IntToStr(port);
			INI->WriteInteger("port COM", "port", port);
			ShowMessage(Popup->Items->Strings[5] + "\n" + Popup->Items->Strings[6] + "\n" + Popup->Items->Strings[7]);
			}
		}
	delete registre1;
	delete registre2;
	}
	*/
}
//valable seulement pour 1 carte Arduino, sinon incrémente...
//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\usbser\Enum -> 0 -> USB\VID_2341&PID_0042\6493633303735151D061
//HKEY_LOCAL_MACHINE\HARDWARE\DEVICEMAP\SERIALCOMM -> \Device\USBSER000 -> COMxx
//---------------------------------------------------------------------------


void __fastcall TInterfaceS2A::Button4Click(TObject *Sender)
{
ShellExecute(0, 0, "http://www.technologiescollege.fr/blockly@rduino", 0, 0 , SW_SHOW );
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::Button3Click(TObject *Sender)
{
port=this->Edit1->Text.ToInt();
INI->WriteInteger("port COM", "port", port);
ofstream fichier_s2a("pymata_cmd.bat", ios::out | ios::trunc);  // ouverture en écriture avec effacement du fichier ouvert
		if(fichier_s2a)
		{
				fichier_s2a << "@echo off\nbreak ON\nrem fichiers BAT et fork créés par Sébastien CANET\nSET currentpath=%~dp1\ncd %currentpath%s2aio\ncls\npython.exe .\\Lib\\site-packages\\pymata_aio\\pymata_iot.py -l 5 -c no_client -comport COM" << port;
				fichier_s2a.close();
		}
		else ShowMessage(Popup->Items->Strings[2]);
ShellExecute(0, 0, "pymata_cmd.bat", 0, 0 , SW_SHOW );
}
//---------------------------------------------------------------------------


void __fastcall TInterfaceS2A::BlocklyArduinoClick(TObject *Sender)
{
Environnement(true);
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::RevenirsurScratchClick(TObject *Sender)
{
Environnement(false);
}
//---------------------------------------------------------------------------

