//---------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN
#include <vcl.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>

#pragma hdrstop

#include <Windows.h>
#include <setupapi.h>   

#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library

using namespace std;  //pour les traductions
#include <tinyxml.h>

#include <IniFiles.hpp> //pour le fichier INI
#include <registry.hpp>

#include "s2aio_scratch2_arduino_killprocess.h"
#include "Detect.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

TInterfaceS2A *InterfaceS2A;

//---------------------------------------------------------------------------

SHELLEXECUTEINFO ShExecS2aio;
SHELLEXECUTEINFO ShExecPymata;
SHELLEXECUTEINFO ShExecScratch;
SHELLEXECUTEINFO ShExecWebServer;

//listes pour récupérer le contenu du fichier INI
int choix_langue;
AnsiString locate_scratch2="Scratch 2.exe";
AnsiString locate_librairie;
AnsiString locate_project;
AnsiString locate_doc;
AnsiString locate_Arduino;
AnsiString arg="";
//listes pour les menus dynamiques
TStringList *Examples = new TStringList;
TStringList *Biblio = new TStringList;
TStringList *Aides = new TStringList;
TStringList *Docs = new TStringList;
TStringList *Extensions1 = new TStringList;
TStringList *Extensions2 = new TStringList;

bool voirDOS = false;

//---------------------------------------------------------------------------
void __fastcall TInterfaceS2A::InitINI()
{
  //écriture dans le fichier INI des variables utiles
  INI->WriteInteger("port COM", "port", 0);      //port par défaut pour initialiser
  INI->WriteString("locate Scratch2", "locate_scratch2", "Scratch 2.exe"); //chemin par défaut pour initialiser
  INI->WriteInteger("langue", "langue", 0); //dans le fichier label.xml, à la balise <Langues>, cela correspond au n° du rang de la langue
  INI->WriteInteger("BlocklyOUScratch", "BoS", false);
  //dossiers à utiliser pour les documents
  INI->WriteString("locate Librairie", "locate_librairie", ExtractFilePath(Application->ExeName) + "bibliotheque\\"); //chemin par défaut pour initialiser
  INI->WriteString("locate Project", "locate_project", ExtractFilePath(Application->ExeName) + "projets\\"); //chemin par défaut pour initialiser
  INI->WriteString("locate Documentation", "locate_doc", ExtractFilePath(Application->ExeName) + "documentation\\"); //chemin par défaut pour initialiser
  INI->WriteString("locate IDE Arduino", "locate_Arduino", ""); //chemin par défaut pour initialiser
}
//---------------------------------------------------------------------------
__fastcall TInterfaceS2A::TInterfaceS2A(TComponent* Owner)
	: TForm(Owner)
{
  //appel à la procédure pour gérer les langues
  AnsiString file = ExtractFilePath(Application->ExeName) + "label.xml";
  langue = new GestionLangue;
  langue->Init(InterfaceS2A->Langue1,file.c_str(),(ptrOnClick)&Langue1Click);
  //après l'initialisation des langues, le système pioche la langue précédemment sélectionnée
  langue->Change(choix_langue);

  INI = new TIniFile(ExtractFilePath(Application->ExeName)+ "s2aio.ini");
  InterfaceS2A->ClientWidth=535;
  InterfaceS2A->Constraints->MinWidth=535;
  InterfaceS2A->Constraints->MaxWidth=535;
  InterfaceS2A->Height=225;

  if (BlocklyArduino->Visible)
	{
	Image_feu_vert->Hint=Popup->Items->Strings[9];
	Image_feu_jaune->Hint=Popup->Items->Strings[10];
	Image_feu_rouge->Hint=Popup->Items->Strings[11];
	}
  if (RevenirsurScratch->Visible)
	{
	Image_feu_vert->Hint=Popup->Items->Strings[12];
	Image_feu_jaune->Hint=Popup->Items->Strings[13];
	Image_feu_rouge->Hint=Popup->Items->Strings[14];
	}

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
  locate_Arduino=INI->ReadString("locate IDE Arduino", "locate_Arduino", "");

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
  Extensions2->Add(".mp4");
  Extensions2->Add(".avi");
  // Appel à SearchDocs, avec pour liste des fichiers les items de Documentation.
  SearchDocs(locate_doc, Extensions2, Docs,2);
  delete Extensions2;

  //vérification d'argument sur la ligne de commande
  arg=GetCommandLine();
  arg=arg.Delete(1,7);  //élimine les 5 premiers caractères : s2aio_
  if(arg=="-i") InitINI();
  Environnement(INI->ReadInteger("BlocklyOUScratch", "BoS", false));  

  ImgConnect->Picture->LoadFromFile("unconnect.bmp");

  TestCarte *ThreadTestArduino=new TestCarte(false);

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
	  InterfaceS2A->Image_feu_vert->Visible=false;
	  InterfaceS2A->Image_feu_vert->Enabled=false;
	  InterfaceS2A->Image_feu_jaune->Visible=false;
	  InterfaceS2A->Image_feu_jaune->Enabled=false;
	  InterfaceS2A->Image_feu_rouge->Visible=false;
	  InterfaceS2A->Image_feu_rouge->Enabled=false;
	  InterfaceS2A->feu_vert_vide->Visible=true;
	  InterfaceS2A->feu_vert_vide->Enabled=false;
	  InterfaceS2A->feu_jaune_vide->Visible=true;
	  InterfaceS2A->feu_jaune_vide->Enabled=false;
	  InterfaceS2A->feu_rouge_vide->Visible=true;
	  InterfaceS2A->feu_rouge_vide->Enabled=false;
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
	  InterfaceS2A->Image_feu_vert->Visible=false;
	  InterfaceS2A->Image_feu_vert->Enabled=false;
	  InterfaceS2A->Image_feu_jaune->Visible=false;
	  InterfaceS2A->Image_feu_jaune->Enabled=false;
	  InterfaceS2A->Image_feu_rouge->Visible=false;
	  InterfaceS2A->Image_feu_rouge->Enabled=false;
	  InterfaceS2A->feu_vert_vide->Visible=true;
	  InterfaceS2A->feu_vert_vide->Enabled=false;
	  InterfaceS2A->feu_jaune_vide->Visible=true;
	  InterfaceS2A->feu_jaune_vide->Enabled=false;
	  InterfaceS2A->feu_rouge_vide->Visible=true;
	  InterfaceS2A->feu_rouge_vide->Enabled=false;
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
//ShellExecute(0, 0, "scratch2.bat", 0, 0 , SW_HIDE );

ShExecScratch.cbSize = sizeof(SHELLEXECUTEINFO);
		ShExecScratch.fMask = SEE_MASK_NOCLOSEPROCESS;
		ShExecScratch.hwnd = NULL;
		ShExecScratch.lpVerb = NULL;
		ShExecScratch.lpFile = locate_scratch2.c_str();
		ShExecScratch.lpParameters = CheminNomFichier.c_str();
		ShExecScratch.lpDirectory = locate_librairie.c_str();
		ShExecScratch.nShow = SW_SHOW;
		ShExecScratch.hInstApp = NULL;
ShellExecuteEx(&ShExecScratch);
PID_scratch = GetProcessId(ShExecScratch.hProcess);

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
				fichier_s2 << "@echo off\nbreak ON\nrem fichiers BAT et fork créés par Sébastien CANET\ncls\nSET currentpath=%~dp1\nSET dossier_scratch="<< '"' << locate_scratch2.c_str() << '"'<< "\nstart %dossier_scratch% " << '"' << locate_librairie.c_str() << "s2aio_Scratch2Arduino.sb2" << '"';
				fichier_s2.close();
		}
		else ShowMessage(Popup->Items->Strings[1]);
//ShellExecute(0, 0, "scratch2.bat", 0, 0 , SW_HIDE );

AnsiString ligne_cmd_arg = locate_librairie + "s2aio_Scratch2Arduino.sb2";
ShExecScratch.cbSize = sizeof(SHELLEXECUTEINFO);
		ShExecScratch.fMask = SEE_MASK_NOCLOSEPROCESS;
		ShExecScratch.hwnd = NULL;
		ShExecScratch.lpVerb = NULL;
		ShExecScratch.lpFile = locate_scratch2.c_str();
		ShExecScratch.lpParameters = ligne_cmd_arg.c_str();
		ShExecScratch.lpDirectory = locate_librairie.c_str();
		ShExecScratch.nShow = SW_SHOW;
		ShExecScratch.hInstApp = NULL;
ShellExecuteEx(&ShExecScratch);
PID_scratch = GetProcessId(ShExecScratch.hProcess);
}
//---------------------------------------------------------------------------
void __fastcall TInterfaceS2A::Button1Click(TObject *Sender)
{
TerminateProcess(ShExecPymata.hProcess, 0);
port=this->Edit1->Text.ToInt();
INI->WriteInteger("port COM", "port", port);
ofstream fichier_s2a("s2aio_cmd.bat", ios::out | ios::trunc);  // ouverture en écriture avec effacement du fichier ouvert
		if(fichier_s2a)
		{
				fichier_s2a << "@echo off\nbreak ON\nrem fichiers BAT et fork créés par Sébastien CANET\nSET currentpath=%~dp1\ncd %currentpath%s2aio\ncls\npython.exe .\\Lib\\site-packages\\s2aio\\__main__.py -l 5 -c no_client -p COM" << port;
				fichier_s2a.close();
		}
		else ShowMessage(Popup->Items->Strings[2]);
//ShellExecute(0, 0, "s2aio_cmd.bat", 0, 0 , SW_SHOW );

AnsiString ligne_cmd = "s2aio\\python.exe ";
AnsiString ligne_cmd_arg = "s2aio\\Lib\\site-packages\\s2aio\\__main__.py -l 5 -c no_client -p COM" + IntToStr(port);
ShExecS2aio.cbSize = sizeof(SHELLEXECUTEINFO);
		ShExecS2aio.fMask = SEE_MASK_NOCLOSEPROCESS;
		ShExecS2aio.hwnd = NULL;
		ShExecS2aio.lpVerb = NULL;
		ShExecS2aio.lpFile = ligne_cmd.c_str();
		ShExecS2aio.lpParameters = ligne_cmd_arg.c_str();
		ShExecS2aio.lpDirectory = ExtractFilePath(Application->ExeName).c_str();
		if (voirDOS) ShExecS2aio.nShow = SW_SHOW;
		if (!voirDOS) ShExecS2aio.nShow = SW_HIDE;
		ShExecS2aio.hInstApp = NULL;
ShellExecuteEx(&ShExecS2aio);
PID_s2aio = GetProcessId(ShExecS2aio.hProcess);
}
//---------------------------------------------------------------------------
void __fastcall TInterfaceS2A::Quitter1Click(TObject *Sender)
{
TerminateProcess(ShExecS2aio.hProcess, 0);
TerminateProcess(ShExecPymata.hProcess, 0);
TerminateProcess(ShExecScratch.hProcess, 0);
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

void __fastcall TInterfaceS2A::DuemilanoveDiecimila328P1Click(TObject *Sender)
{
port=this->Edit1->Text.ToInt();
INI->WriteInteger("port COM", "port", port);
ofstream fichier_s2(".\\tools\\flash_duemilanove.bat", ios::out | ios::trunc);  // ouverture en écriture avec effacement du fichier ouvert
		if(fichier_s2)
		{
				fichier_s2 << "@echo off\nbreak ON\nrem fichiers BAT et fork créés par Sébastien CANET\nSET currentpath=%~dp1\ncd %currentpath%tools\ncls\n.\\avrdude -Cavrdude.conf -v -patmega328p -carduino -P\\" << "\\" << ".\\COM" << port << " -b57600 -D -V -Uflash:w:s2aio-FirmataPlus.Duemilanove.hex:i" << "\npause";
				fichier_s2.close();
		}
		else ShowMessage(Popup->Items->Strings[1]);
ShellExecute(0, 0, ".\\tools\\flash_duemilanove.bat", 0, 0 , SW_SHOW );
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

void __fastcall TInterfaceS2A::Button4Click(TObject *Sender)
{
//ShellExecute(0, 0, "http://www.technologiescollege.fr/blockly@rduino", 0, 0 , SW_SHOW );
if (locate_Arduino!="") {
	ofstream fichier_web("arduino_web_server.bat", ios::out | ios::trunc);  // ouverture en écriture avec effacement du fichier ouvert
		if(fichier_web)
		{
				fichier_web << "@echo off\nbreak ON\nrem fichiers BAT créés par Sébastien CANET\nSET currentpath=%~dp0\nSET arduinopath=" << locate_Arduino.c_str() << "\ncd %arduinopath%\n" << (locate_Arduino.SubString(0,2)).c_str() << "\narduino.exe %currentpath%s2aio\\blocklytemp\\blocklytemp.ino";
				fichier_web.close();
		}
	} else {
		ofstream fichier_web("arduino_web_server.bat", ios::out | ios::trunc);
		fichier_web.close();
		}
AnsiString ligne_cmd = "s2aio\\python.exe ";
AnsiString ligne_cmd_arg = "s2aio\\arduino_web_server.py";
ShExecWebServer.cbSize = sizeof(SHELLEXECUTEINFO);
		ShExecWebServer.fMask = SEE_MASK_NOCLOSEPROCESS;
		ShExecWebServer.hwnd = NULL;
		ShExecWebServer.lpVerb = NULL;
		ShExecWebServer.lpFile = ligne_cmd.c_str();
		ShExecWebServer.lpParameters = ligne_cmd_arg.c_str();
		ShExecWebServer.lpDirectory = ExtractFilePath(Application->ExeName).c_str();
		if (voirDOS) ShExecWebServer.nShow = SW_SHOW;
		if (!voirDOS) ShExecWebServer.nShow = SW_HIDE;
		ShExecWebServer.hInstApp = NULL;
ShellExecuteEx(&ShExecWebServer);
PID_webserver = GetProcessId(ShExecWebServer.hProcess);
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::Button3Click(TObject *Sender)
{
TerminateProcess(ShExecS2aio.hProcess, 0);
port=this->Edit1->Text.ToInt();
INI->WriteInteger("port COM", "port", port);
ofstream fichier_s2a("pymata_cmd.bat", ios::out | ios::trunc);  // ouverture en écriture avec effacement du fichier ouvert
		if(fichier_s2a)
		{
				fichier_s2a << "@echo off\nbreak ON\nrem fichiers BAT et fork créés par Sébastien CANET\nSET currentpath=%~dp1\ncd %currentpath%s2aio\ncls\npython.exe .\\Lib\\site-packages\\pymata_aio\\pymata_iot.py -l 5 -c no_client -comport COM" << port;
				fichier_s2a.close();
		}
		else ShowMessage(Popup->Items->Strings[2]);
//ShellExecute(0, 0, "pymata_cmd.bat", 0, 0 , SW_SHOW );
AnsiString ligne_cmd = "s2aio\\python.exe ";
AnsiString ligne_cmd_arg = "s2aio\\Lib\\site-packages\\pymata_aio\\pymata_iot.py -l 5 -c no_client -comport COM" + IntToStr(port);
ShExecPymata.cbSize = sizeof(SHELLEXECUTEINFO);
		ShExecPymata.fMask = SEE_MASK_NOCLOSEPROCESS;
		ShExecPymata.hwnd = NULL;
		ShExecPymata.lpVerb = NULL;
		ShExecPymata.lpFile = ligne_cmd.c_str();
		ShExecPymata.lpParameters = ligne_cmd_arg.c_str();
		ShExecPymata.lpDirectory = ExtractFilePath(Application->ExeName).c_str();
		if (voirDOS) ShExecPymata.nShow = SW_SHOW;
		if (!voirDOS) ShExecPymata.nShow = SW_HIDE;
		ShExecPymata.hInstApp = NULL;
ShellExecuteEx(&ShExecPymata);
PID_pymata = GetProcessId(ShExecPymata.hProcess);
}
//---------------------------------------------------------------------------


void __fastcall TInterfaceS2A::BlocklyArduinoClick(TObject *Sender)
{
TerminateProcess(ShExecS2aio.hProcess, 0);
TerminateProcess(ShExecScratch.hProcess, 0);
INI->WriteInteger("BlocklyOUScratch", "BoS", true); 
Environnement(true);
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::RevenirsurScratchClick(TObject *Sender)
{
TerminateProcess(ShExecPymata.hProcess, 0);
TerminateProcess(ShExecWebServer.hProcess, 0);
INI->WriteInteger("BlocklyOUScratch", "BoS", false);
Environnement(false);
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::Image_feu_rougeClick(TObject *Sender)
{
TerminateProcess(ShExecS2aio.hProcess, 0);
TerminateProcess(ShExecPymata.hProcess, 0);
}
//---------------------------------------------------------------------------


void __fastcall TInterfaceS2A::Image_feu_jauneClick(TObject *Sender)
{
TerminateProcess(ShExecScratch.hProcess, 0);
TerminateProcess(ShExecWebServer.hProcess, 0);

}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::Image_feu_vertClick(TObject *Sender)
{
TerminateProcess(ShExecS2aio.hProcess, 0);
TerminateProcess(ShExecPymata.hProcess, 0);
TerminateProcess(ShExecScratch.hProcess, 0);
TerminateProcess(ShExecWebServer.hProcess, 0);
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::FormClose(TObject *Sender, TCloseAction &Action)
{
TerminateProcess(ShExecS2aio.hProcess, 0);
TerminateProcess(ShExecPymata.hProcess, 0);
TerminateProcess(ShExecScratch.hProcess, 0);      
TerminateProcess(ShExecWebServer.hProcess, 0);
DeleteFile(ExtractFilePath(Application->ExeName)+ "s2aio_debug.log");
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::DebugMenuClick(TObject *Sender)
{
WSADATA wsa;
SOCKET s;
struct sockaddr_in server;
char *message, server_reply[2000];
char *error_message = "";
int recv_size;
//supprime ancien fichier log
DeleteFile(ExtractFilePath(Application->ExeName)+ "s2aio_debug.log");

//ouverture du socket
if (WSAStartup(MAKEWORD(2,2),&wsa) != 0) error_message = "Failed. Error Code : " + WSAGetLastError();
if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET) error_message = strcat(error_message, "Could not create socket : " + WSAGetLastError());
	else error_message = strcat(error_message, "Socket created.\n");

//definition server
server.sin_addr.s_addr = inet_addr("127.0.0.1");
server.sin_family = AF_INET;
if (BlocklyArduino->Visible) server.sin_port = htons( 50209 );
if (RevenirsurScratch->Visible) server.sin_port = htons( 9000 );

error_message = strcat(error_message, IntToStr(server.sin_port).c_str());

//Connect to remote server
if (connect(s , (struct sockaddr *)&server , sizeof(server)) < 0) error_message = strcat(error_message, "\nconnect error\n");
	else error_message = strcat(error_message, "\nConnected\n");

//envoi du poll pour vérifier que ça fonctionne
message = "GET /poll HTTP/1.1\r\n\r\n";
if( send(s , message , strlen(message) , 0) < 0) error_message = strcat(error_message, "Send failed\n");
	else error_message = strcat(error_message, "Data Send\n");

//réception de données
if((recv_size = recv(s , server_reply , 2000 , 0)) == SOCKET_ERROR) error_message = strcat(error_message, "receive failed\n");
	else error_message = strcat(error_message, "Reply received\n");

//Add a NULL terminating character to make it a proper string before printing
server_reply[recv_size] = '\0';
ofstream fichier_debug("s2aio_debug.log", ios::out | ios::trunc);  // ouverture en écriture avec effacement du fichier ouvert
if(fichier_debug)
		{
				fichier_debug << error_message;
				fichier_debug.close();
		}
		else ShowMessage(Popup->Items->Strings[1]);
error_message = "";
ShellExecute(0, "open", "notepad.exe", (ExtractFilePath(Application->ExeName)+ "s2aio_debug.log").c_str(), 0 , SW_SHOW );
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::s2aio_windowClick(TObject *Sender)
{
if (voirDOS)
	{
	voirDOS = false;
	s2aio_window->Checked = false;
	}
	else {
		voirDOS = true;
		s2aio_window->Checked = true;
		}
}
//---------------------------------------------------------------------------
void __fastcall TInterfaceS2A::LocaliserIDEClick(TObject *Sender)
{
if (Open_Arduino->Execute()==true) locate_Arduino=Open_Arduino->FileName.c_str();
//écriture dans le fichier INI
INI->WriteString("locate IDE Arduino", "locate_Arduino", locate_Arduino);
}
//---------------------------------------------------------------------------

void __fastcall TInterfaceS2A::Edit1Change(TObject *Sender)
{
if (Edit1->Text=="0") ShowMessage(Popup->Items->Strings[5] + "\n" + Popup->Items->Strings[6] + "\n" + Popup->Items->Strings[7]);

}
//---------------------------------------------------------------------------

