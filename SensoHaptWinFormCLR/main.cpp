/////////////////////////////////////////////////////////////////////////////
//Thesisproject
//KULeuven
//Programmers: Gilles Meyhi & Elias Stalpaert
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"


#include "ExperimentUI.h"

using namespace System;
using namespace System::Diagnostics;

[STAThread]
int main()
{
	//Start the main windows form
	System::Windows::Forms::Application::EnableVisualStyles();
	System::Windows::Forms::Application::SetCompatibleTextRenderingDefault(false);
	System::Windows::Forms::Application::Run(gcnew SensoHaptWinFormCLR::ExperimentUI());
	return 0;
}
