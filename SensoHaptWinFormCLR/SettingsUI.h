#pragma once

#include "forms_headers.h"
#include "common_variables.h"
#include "pugixml\pugixml.hpp"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace pugi;

namespace SensoHaptWinFormCLR 
{
	//Settings window for manual configuration of output paths and hardware details
	public ref class SettingsUI : public System::Windows::Forms::Form
	{
	public:
		SettingsUI(void)
		{
			InitializeComponent();
			InitializeProgrammerComponents();
		}
	protected:
		~SettingsUI()
		{
			if (components) delete components;
		}
	private: 
		System::ComponentModel::Container^ components;

		System::Windows::Forms::TextBox^ hdNameTextBox;
		System::Windows::Forms::TextBox^ daqNameTextBox;
		System::Windows::Forms::TextBox^ leftCalTextBox;
		System::Windows::Forms::TextBox^ rightCalTextBox;
		System::Windows::Forms::TextBox^ trialDataTextBox;
		System::Windows::Forms::TextBox^ answersTextBox;
		System::Windows::Forms::TextBox^ experimentDetailTextBox;

		System::Windows::Forms::Button^ leftCalBrowseButton;
		System::Windows::Forms::Button^ rightCalBrowseButton;
		System::Windows::Forms::Button^ browseTrialDataButton;
		System::Windows::Forms::Button^ browseAnswersButton;
		System::Windows::Forms::Button^ browseExperimentDetailButton;
		System::Windows::Forms::Button^ resetDefaultsButton;

		OpenFileDialog^ openFileDialog;
		FolderBrowserDialog^ folderBrowserDialog;
		System::String^ configXmlPath;
		msclr::interop::marshal_context context;
		
#pragma region Windows Form Designer generated code
		void InitializeComponent(void)
		{
			System::Windows::Forms::Label^  hdNameLabel;
			System::Windows::Forms::Label^  daqNameLabel;
			System::Windows::Forms::Label^  leftCalLabel;
			System::Windows::Forms::Label^  rightCalLabel;
			System::Windows::Forms::Label^  trialDataLabel;
			System::Windows::Forms::Label^  answersLabel;
			System::Windows::Forms::Label^  experimentDetailLabel;
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(SettingsUI::typeid));
			this->hdNameTextBox = (gcnew System::Windows::Forms::TextBox());
			this->daqNameTextBox = (gcnew System::Windows::Forms::TextBox());
			this->leftCalTextBox = (gcnew System::Windows::Forms::TextBox());
			this->leftCalBrowseButton = (gcnew System::Windows::Forms::Button());
			this->rightCalTextBox = (gcnew System::Windows::Forms::TextBox());
			this->rightCalBrowseButton = (gcnew System::Windows::Forms::Button());
			this->trialDataTextBox = (gcnew System::Windows::Forms::TextBox());
			this->browseTrialDataButton = (gcnew System::Windows::Forms::Button());
			this->answersTextBox = (gcnew System::Windows::Forms::TextBox());
			this->browseAnswersButton = (gcnew System::Windows::Forms::Button());
			this->experimentDetailTextBox = (gcnew System::Windows::Forms::TextBox());
			this->browseExperimentDetailButton = (gcnew System::Windows::Forms::Button());
			this->resetDefaultsButton = (gcnew System::Windows::Forms::Button());
			hdNameLabel = (gcnew System::Windows::Forms::Label());
			daqNameLabel = (gcnew System::Windows::Forms::Label());
			leftCalLabel = (gcnew System::Windows::Forms::Label());
			rightCalLabel = (gcnew System::Windows::Forms::Label());
			trialDataLabel = (gcnew System::Windows::Forms::Label());
			answersLabel = (gcnew System::Windows::Forms::Label());
			experimentDetailLabel = (gcnew System::Windows::Forms::Label());
			this->SuspendLayout();
			// 
			// hdNameLabel
			// 
			hdNameLabel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			hdNameLabel->AutoSize = true;
			hdNameLabel->Location = System::Drawing::Point(10, 15);
			hdNameLabel->Name = L"hdNameLabel";
			hdNameLabel->Size = System::Drawing::Size(106, 13);
			hdNameLabel->TabIndex = 1;
			hdNameLabel->Text = L"Haptic Device Name";
			// 
			// daqNameLabel
			// 
			daqNameLabel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			daqNameLabel->AutoSize = true;
			daqNameLabel->Location = System::Drawing::Point(10, 81);
			daqNameLabel->Name = L"daqNameLabel";
			daqNameLabel->Size = System::Drawing::Size(61, 13);
			daqNameLabel->TabIndex = 2;
			daqNameLabel->Text = L"DAQ Name";
			// 
			// leftCalLabel
			// 
			leftCalLabel->AutoSize = true;
			leftCalLabel->Location = System::Drawing::Point(10, 147);
			leftCalLabel->Name = L"leftCalLabel";
			leftCalLabel->Size = System::Drawing::Size(157, 13);
			leftCalLabel->TabIndex = 4;
			leftCalLabel->Text = L"Left Sensor Calibration File Path";
			// 
			// rightCalLabel
			// 
			rightCalLabel->AutoSize = true;
			rightCalLabel->Location = System::Drawing::Point(10, 213);
			rightCalLabel->Name = L"rightCalLabel";
			rightCalLabel->Size = System::Drawing::Size(164, 13);
			rightCalLabel->TabIndex = 7;
			rightCalLabel->Text = L"Right Sensor Calibration File Path";
			// 
			// trialDataLabel
			// 
			trialDataLabel->AutoSize = true;
			trialDataLabel->Location = System::Drawing::Point(12, 283);
			trialDataLabel->Name = L"trialDataLabel";
			trialDataLabel->Size = System::Drawing::Size(120, 13);
			trialDataLabel->TabIndex = 10;
			trialDataLabel->Text = L"Trial Data Output Folder";
			// 
			// answersLabel
			// 
			answersLabel->AutoSize = true;
			answersLabel->Location = System::Drawing::Point(12, 355);
			answersLabel->Name = L"answersLabel";
			answersLabel->Size = System::Drawing::Size(114, 13);
			answersLabel->TabIndex = 13;
			answersLabel->Text = L"Answers Output Folder";
			// 
			// experimentDetailLabel
			// 
			experimentDetailLabel->AutoSize = true;
			experimentDetailLabel->Location = System::Drawing::Point(10, 427);
			experimentDetailLabel->Name = L"experimentDetailLabel";
			experimentDetailLabel->Size = System::Drawing::Size(161, 13);
			experimentDetailLabel->TabIndex = 16;
			experimentDetailLabel->Text = L"Experiment Details Output Folder";
			// 
			// hdNameTextBox
			// 
			this->hdNameTextBox->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->hdNameTextBox->Location = System::Drawing::Point(27, 40);
			this->hdNameTextBox->Name = L"hdNameTextBox";
			this->hdNameTextBox->Size = System::Drawing::Size(665, 20);
			this->hdNameTextBox->TabIndex = 0;
			// 
			// daqNameTextBox
			// 
			this->daqNameTextBox->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->daqNameTextBox->Location = System::Drawing::Point(27, 107);
			this->daqNameTextBox->Name = L"daqNameTextBox";
			this->daqNameTextBox->Size = System::Drawing::Size(665, 20);
			this->daqNameTextBox->TabIndex = 3;
			// 
			// leftCalTextBox
			// 
			this->leftCalTextBox->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->leftCalTextBox->Location = System::Drawing::Point(27, 172);
			this->leftCalTextBox->Name = L"leftCalTextBox";
			this->leftCalTextBox->Size = System::Drawing::Size(609, 20);
			this->leftCalTextBox->TabIndex = 5;
			// 
			// leftCalBrowseButton
			// 
			this->leftCalBrowseButton->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->leftCalBrowseButton->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->leftCalBrowseButton->Location = System::Drawing::Point(656, 172);
			this->leftCalBrowseButton->Name = L"leftCalBrowseButton";
			this->leftCalBrowseButton->Size = System::Drawing::Size(47, 20);
			this->leftCalBrowseButton->TabIndex = 6;
			this->leftCalBrowseButton->Text = L"...";
			this->leftCalBrowseButton->UseVisualStyleBackColor = true;
			this->leftCalBrowseButton->Click += gcnew System::EventHandler(this, &SettingsUI::leftCalBrowseButton_Click);
			// 
			// rightCalTextBox
			// 
			this->rightCalTextBox->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->rightCalTextBox->Location = System::Drawing::Point(27, 239);
			this->rightCalTextBox->Name = L"rightCalTextBox";
			this->rightCalTextBox->Size = System::Drawing::Size(609, 20);
			this->rightCalTextBox->TabIndex = 8;
			// 
			// rightCalBrowseButton
			// 
			this->rightCalBrowseButton->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->rightCalBrowseButton->Location = System::Drawing::Point(656, 239);
			this->rightCalBrowseButton->Name = L"rightCalBrowseButton";
			this->rightCalBrowseButton->Size = System::Drawing::Size(47, 20);
			this->rightCalBrowseButton->TabIndex = 9;
			this->rightCalBrowseButton->Text = L"...";
			this->rightCalBrowseButton->UseVisualStyleBackColor = true;
			this->rightCalBrowseButton->Click += gcnew System::EventHandler(this, &SettingsUI::rightCalBrowseButton_Click);
			// 
			// trialDataTextBox
			// 
			this->trialDataTextBox->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->trialDataTextBox->Location = System::Drawing::Point(27, 310);
			this->trialDataTextBox->Name = L"trialDataTextBox";
			this->trialDataTextBox->Size = System::Drawing::Size(609, 20);
			this->trialDataTextBox->TabIndex = 11;
			// 
			// browseTrialDataButton
			// 
			this->browseTrialDataButton->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->browseTrialDataButton->Location = System::Drawing::Point(656, 310);
			this->browseTrialDataButton->Name = L"browseTrialDataButton";
			this->browseTrialDataButton->Size = System::Drawing::Size(47, 20);
			this->browseTrialDataButton->TabIndex = 12;
			this->browseTrialDataButton->Text = L"...";
			this->browseTrialDataButton->UseVisualStyleBackColor = true;
			this->browseTrialDataButton->Click += gcnew System::EventHandler(this, &SettingsUI::browseTrialDataButton_Click);
			// 
			// answersTextBox
			// 
			this->answersTextBox->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->answersTextBox->Location = System::Drawing::Point(27, 383);
			this->answersTextBox->Name = L"answersTextBox";
			this->answersTextBox->Size = System::Drawing::Size(609, 20);
			this->answersTextBox->TabIndex = 14;
			// 
			// browseAnswersButton
			// 
			this->browseAnswersButton->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->browseAnswersButton->Location = System::Drawing::Point(656, 383);
			this->browseAnswersButton->Name = L"browseAnswersButton";
			this->browseAnswersButton->Size = System::Drawing::Size(47, 20);
			this->browseAnswersButton->TabIndex = 15;
			this->browseAnswersButton->Text = L"...";
			this->browseAnswersButton->UseVisualStyleBackColor = true;
			this->browseAnswersButton->Click += gcnew System::EventHandler(this, &SettingsUI::browseAnswersButton_Click);
			// 
			// experimentDetailTextBox
			// 
			this->experimentDetailTextBox->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->experimentDetailTextBox->Location = System::Drawing::Point(27, 454);
			this->experimentDetailTextBox->Name = L"experimentDetailTextBox";
			this->experimentDetailTextBox->Size = System::Drawing::Size(609, 20);
			this->experimentDetailTextBox->TabIndex = 17;
			// 
			// browseExperimentDetailButton
			// 
			this->browseExperimentDetailButton->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->browseExperimentDetailButton->Location = System::Drawing::Point(656, 454);
			this->browseExperimentDetailButton->Name = L"browseExperimentDetailButton";
			this->browseExperimentDetailButton->Size = System::Drawing::Size(47, 20);
			this->browseExperimentDetailButton->TabIndex = 18;
			this->browseExperimentDetailButton->Text = L"...";
			this->browseExperimentDetailButton->UseVisualStyleBackColor = true;
			this->browseExperimentDetailButton->Click += gcnew System::EventHandler(this, &SettingsUI::browseExperimentDetailButton_Click);
			// 
			// resetDefaultsButton
			// 
			this->resetDefaultsButton->Anchor = System::Windows::Forms::AnchorStyles::Bottom;
			this->resetDefaultsButton->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 12, System::Drawing::FontStyle::Regular,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->resetDefaultsButton->Location = System::Drawing::Point(256, 495);
			this->resetDefaultsButton->Name = L"resetDefaultsButton";
			this->resetDefaultsButton->Size = System::Drawing::Size(196, 54);
			this->resetDefaultsButton->TabIndex = 19;
			this->resetDefaultsButton->Text = L"Reset Defaults";
			this->resetDefaultsButton->UseVisualStyleBackColor = true;
			this->resetDefaultsButton->Click += gcnew System::EventHandler(this, &SettingsUI::resetDefaultsButton_Click);
			// 
			// SettingsUI
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(748, 561);
			this->Controls->Add(this->resetDefaultsButton);
			this->Controls->Add(this->browseExperimentDetailButton);
			this->Controls->Add(this->experimentDetailTextBox);
			this->Controls->Add(experimentDetailLabel);
			this->Controls->Add(this->browseAnswersButton);
			this->Controls->Add(this->answersTextBox);
			this->Controls->Add(answersLabel);
			this->Controls->Add(this->browseTrialDataButton);
			this->Controls->Add(this->trialDataTextBox);
			this->Controls->Add(trialDataLabel);
			this->Controls->Add(this->rightCalBrowseButton);
			this->Controls->Add(this->rightCalTextBox);
			this->Controls->Add(rightCalLabel);
			this->Controls->Add(this->leftCalBrowseButton);
			this->Controls->Add(this->leftCalTextBox);
			this->Controls->Add(leftCalLabel);
			this->Controls->Add(this->daqNameTextBox);
			this->Controls->Add(daqNameLabel);
			this->Controls->Add(hdNameLabel);
			this->Controls->Add(this->hdNameTextBox);
			this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
			this->MinimumSize = System::Drawing::Size(280, 600);
			this->Name = L"SettingsUI";
			this->Text = L"Settings";
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &SettingsUI::SettingsUI_FormClosing);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

		void InitializeProgrammerComponents()
		{
			//Initialize openfiledialog for browsing calibration files
			openFileDialog = gcnew OpenFileDialog{};
			openFileDialog->InitialDirectory = Application::StartupPath;
			openFileDialog->Filter = "Calibration files (*.cal)|*.cal";
			openFileDialog->FilterIndex = 1;
			openFileDialog->RestoreDirectory = false;

			//Initialize folderbrowserdialog for browsing folders
			folderBrowserDialog = gcnew FolderBrowserDialog{};
			folderBrowserDialog->ShowNewFolderButton = true;

			//Read current values from XML and display in textboxes
			configXmlPath = gcnew System::String(Application::StartupPath + L"\\UserSettings\\experimentXmlPath.xml");
			std::wstring configXmlPathStd = context.marshal_as<std::wstring>(configXmlPath);
			xml_document doc;

			if (!(doc.load_file(configXmlPathStd.c_str()))) ErrorExit(L"config xml corrupted, please restart application");

			hdNameTextBox->Text = gcnew System::String(doc.child("HapticDeviceName").attribute("name").as_string());
			daqNameTextBox->Text = gcnew System::String(doc.child("DaqName").attribute("name").as_string());
			leftCalTextBox->Text = gcnew System::String(doc.child("LeftSensorCalibration").attribute("path").as_string());
			rightCalTextBox->Text = gcnew System::String(doc.child("RightSensorCalibration").attribute("path").as_string());
			trialDataTextBox->Text = gcnew System::String(doc.child("TrialDataPath").attribute("path").as_string());
			answersTextBox->Text = gcnew System::String(doc.child("AnswersPath").attribute("path").as_string());
			experimentDetailTextBox->Text = gcnew System::String(doc.child("ExperimentDetailPath").attribute("path").as_string());
		}

		System::Void SettingsUI_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e) 
		{
			//Save current textbox values to XML
			xml_document doc;
			std::wstring configXmlPathStd = context.marshal_as<std::wstring>(configXmlPath);

			doc.load_file(configXmlPathStd.c_str());

			doc.child("HapticDeviceName").attribute("name").set_value(context.marshal_as<std::string>(hdNameTextBox->Text).c_str());
			doc.child("DaqName").attribute("name").set_value(context.marshal_as<std::string>(daqNameTextBox->Text).c_str());
			doc.child("LeftSensorCalibration").attribute("path").set_value(context.marshal_as<std::string>(leftCalTextBox->Text).c_str());
			doc.child("RightSensorCalibration").attribute("path").set_value(context.marshal_as<std::string>(rightCalTextBox->Text).c_str());
			doc.child("TrialDataPath").attribute("path").set_value(context.marshal_as<std::string>(trialDataTextBox->Text).c_str());
			doc.child("AnswersPath").attribute("path").set_value(context.marshal_as<std::string>(answersTextBox->Text).c_str());
			doc.child("ExperimentDetailPath").attribute("path").set_value(context.marshal_as<std::string>(experimentDetailTextBox->Text).c_str());

			doc.save_file(configXmlPathStd.c_str());
		}
		System::Void resetDefaultsButton_Click(System::Object^  sender, System::EventArgs^  e) 
		{
			//Set textbox values to default values
			hdNameTextBox->Text = DEFAULT_HD_NAME;
			daqNameTextBox->Text = DEFAULT_DAQ_NAME;
			leftCalTextBox->Text = Application::StartupPath + L"\\SensorCalibrations\\FT8568.cal";
			rightCalTextBox->Text = Application::StartupPath + L"\\SensorCalibrations\\FT8569.cal";
			trialDataTextBox->Text = Application::StartupPath + L"\\TrialData";
			answersTextBox->Text = Application::StartupPath + L"\\Answers";
			experimentDetailTextBox->Text = Application::StartupPath + L"\\ExperimentDetails";
		}
		System::Void leftCalBrowseButton_Click(System::Object^  sender, System::EventArgs^  e) 
		{
			if (openFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK) leftCalTextBox->Text = openFileDialog->FileName;
		}
		System::Void rightCalBrowseButton_Click(System::Object^  sender, System::EventArgs^  e) 
		{
			//openfiledialog for left .cal file
			if (openFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK) rightCalTextBox->Text = openFileDialog->FileName;
		}
		System::Void browseTrialDataButton_Click(System::Object^  sender, System::EventArgs^  e) 
		{
			folderBrowserDialog->Description = gcnew System::String(L"Please choose a Trial Data output directory");
			folderBrowserDialog->SelectedPath = Application::StartupPath + L"\\TrialData";
			if (folderBrowserDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK) trialDataTextBox->Text = folderBrowserDialog->SelectedPath;
		}
		System::Void browseAnswersButton_Click(System::Object^  sender, System::EventArgs^  e) 
		{
			folderBrowserDialog->Description = gcnew System::String(L"Please choose an Answers output directory");
			folderBrowserDialog->SelectedPath = Application::StartupPath + L"\\Answers";
			if (folderBrowserDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK) answersTextBox->Text = folderBrowserDialog->SelectedPath;
		}
		System::Void browseExperimentDetailButton_Click(System::Object^  sender, System::EventArgs^  e) 
		{
			folderBrowserDialog->Description = gcnew System::String(L"Please choose an Experiment Details output directory");
			folderBrowserDialog->SelectedPath = Application::StartupPath + L"\\ExperimentDetails";
			if (folderBrowserDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK) experimentDetailTextBox->Text = folderBrowserDialog->SelectedPath;
		}
	};
}
