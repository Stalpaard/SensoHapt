#pragma once

#include "parser_headers.h"
#include "forms_headers.h"
#include "common_variables.h"
#include "pugixml\pugixml.hpp"

using namespace pugi;
using namespace System;
using namespace System::IO;
using namespace System::Diagnostics;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Configuration;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Xml;
using namespace System::Xml::Schema;
using namespace System::Xml::Serialization;

DWORD WINAPI experimentMain(LPVOID lpParam);

//Global variables
std::shared_ptr<ExperimentInfo> ExperimentCommon::expInfo = nullptr;

namespace SensoHaptWinFormCLR 
{

	//Main UI window for running the experiment
	public ref class ExperimentUI : public System::Windows::Forms::Form
	{
	public:
		ExperimentUI(void)
		{
			InitializeComponent();
			InitializeProgrammerComponents();
		}

	protected:
		~ExperimentUI()
		{
			if (components) delete components;
		}
	private: 
		System::ComponentModel::Container^ components;
		System::Windows::Forms::Button^  runButton;
		System::Windows::Forms::Button^  settingsButton;
		System::Windows::Forms::Button^  loadExperimentButton;
		System::Windows::Forms::StatusStrip^  statusStrip1;
		System::Windows::Forms::ToolStripStatusLabel^  experimentStatusLabel;
		System::Windows::Forms::Label^  statusLabel;
		System::ComponentModel::BackgroundWorker^  backgroundWorker1;
		System::Windows::Forms::TreeView^  treeXml;
		System::Windows::Forms::NumericUpDown^  subjectNumber;

		OpenFileDialog^ openFileDialog;
		msclr::interop::marshal_context context;
		System::String^ configXmlPath;
		HANDLE experiment_thread_handle;

#pragma region Windows Form Designer generated code
		void InitializeComponent(void)
		{
			System::Windows::Forms::Label^  subjectLabel;
			System::Windows::Forms::Label^  overviewLabel;
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(ExperimentUI::typeid));
			this->runButton = (gcnew System::Windows::Forms::Button());
			this->settingsButton = (gcnew System::Windows::Forms::Button());
			this->subjectNumber = (gcnew System::Windows::Forms::NumericUpDown());
			this->loadExperimentButton = (gcnew System::Windows::Forms::Button());
			this->statusStrip1 = (gcnew System::Windows::Forms::StatusStrip());
			this->experimentStatusLabel = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->statusLabel = (gcnew System::Windows::Forms::Label());
			this->backgroundWorker1 = (gcnew System::ComponentModel::BackgroundWorker());
			this->treeXml = (gcnew System::Windows::Forms::TreeView());
			subjectLabel = (gcnew System::Windows::Forms::Label());
			overviewLabel = (gcnew System::Windows::Forms::Label());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->subjectNumber))->BeginInit();
			this->statusStrip1->SuspendLayout();
			this->SuspendLayout();
			// 
			// subjectLabel
			// 
			subjectLabel->AutoSize = true;
			subjectLabel->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			subjectLabel->Location = System::Drawing::Point(12, 14);
			subjectLabel->Name = L"subjectLabel";
			subjectLabel->Size = System::Drawing::Size(121, 20);
			subjectLabel->TabIndex = 3;
			subjectLabel->Text = L"Subject number";
			// 
			// overviewLabel
			// 
			overviewLabel->AutoSize = true;
			overviewLabel->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			overviewLabel->Location = System::Drawing::Point(16, 106);
			overviewLabel->Name = L"overviewLabel";
			overviewLabel->Size = System::Drawing::Size(162, 17);
			overviewLabel->TabIndex = 9;
			overviewLabel->Text = L"Loaded Config Overview";
			// 
			// runButton
			// 
			this->runButton->Anchor = System::Windows::Forms::AnchorStyles::Top;
			this->runButton->Enabled = false;
			this->runButton->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->runButton->Location = System::Drawing::Point(437, 12);
			this->runButton->Name = L"runButton";
			this->runButton->Size = System::Drawing::Size(153, 87);
			this->runButton->TabIndex = 0;
			this->runButton->Text = L"Run";
			this->runButton->UseVisualStyleBackColor = true;
			this->runButton->Click += gcnew System::EventHandler(this, &ExperimentUI::runButton_Click);
			// 
			// settingsButton
			// 
			this->settingsButton->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->settingsButton->Location = System::Drawing::Point(16, 47);
			this->settingsButton->Name = L"settingsButton";
			this->settingsButton->Size = System::Drawing::Size(133, 54);
			this->settingsButton->TabIndex = 1;
			this->settingsButton->Text = L"Settings";
			this->settingsButton->UseVisualStyleBackColor = true;
			this->settingsButton->Click += gcnew System::EventHandler(this, &ExperimentUI::settingsButton_Click);
			// 
			// subjectNumber
			// 
			this->subjectNumber->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->subjectNumber->Location = System::Drawing::Point(139, 12);
			this->subjectNumber->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 65535, 0, 0, 0 });
			this->subjectNumber->Name = L"subjectNumber";
			this->subjectNumber->Size = System::Drawing::Size(55, 26);
			this->subjectNumber->TabIndex = 4;
			// 
			// loadExperimentButton
			// 
			this->loadExperimentButton->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->loadExperimentButton->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Regular,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->loadExperimentButton->Location = System::Drawing::Point(887, 12);
			this->loadExperimentButton->Name = L"loadExperimentButton";
			this->loadExperimentButton->Size = System::Drawing::Size(116, 42);
			this->loadExperimentButton->TabIndex = 5;
			this->loadExperimentButton->Text = L"Load Experiment";
			this->loadExperimentButton->UseVisualStyleBackColor = true;
			this->loadExperimentButton->Click += gcnew System::EventHandler(this, &ExperimentUI::loadExperimentButton_Click);
			// 
			// statusStrip1
			// 
			this->statusStrip1->AutoSize = false;
			this->statusStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) { this->experimentStatusLabel });
			this->statusStrip1->Location = System::Drawing::Point(0, 601);
			this->statusStrip1->Name = L"statusStrip1";
			this->statusStrip1->Size = System::Drawing::Size(1083, 22);
			this->statusStrip1->TabIndex = 6;
			this->statusStrip1->Text = L"statusStrip1";
			// 
			// experimentStatusLabel
			// 
			this->experimentStatusLabel->Name = L"experimentStatusLabel";
			this->experimentStatusLabel->Size = System::Drawing::Size(109, 17);
			this->experimentStatusLabel->Text = L"Experiment Loaded";
			// 
			// statusLabel
			// 
			this->statusLabel->Anchor = System::Windows::Forms::AnchorStyles::Top;
			this->statusLabel->AutoSize = true;
			this->statusLabel->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->statusLabel->Location = System::Drawing::Point(635, 58);
			this->statusLabel->Name = L"statusLabel";
			this->statusLabel->Padding = System::Windows::Forms::Padding(0, 0, 10, 0);
			this->statusLabel->Size = System::Drawing::Size(66, 17);
			this->statusLabel->TabIndex = 7;
			this->statusLabel->Text = L"Status: ";
			// 
			// treeXml
			// 
			this->treeXml->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->treeXml->Location = System::Drawing::Point(16, 126);
			this->treeXml->Name = L"treeXml";
			this->treeXml->Size = System::Drawing::Size(1055, 472);
			this->treeXml->TabIndex = 8;
			// 
			// ExperimentUI
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->AutoSizeMode = System::Windows::Forms::AutoSizeMode::GrowAndShrink;
			this->ClientSize = System::Drawing::Size(1083, 623);
			this->Controls->Add(overviewLabel);
			this->Controls->Add(this->treeXml);
			this->Controls->Add(this->statusLabel);
			this->Controls->Add(this->statusStrip1);
			this->Controls->Add(this->loadExperimentButton);
			this->Controls->Add(this->subjectNumber);
			this->Controls->Add(subjectLabel);
			this->Controls->Add(this->settingsButton);
			this->Controls->Add(this->runButton);
			this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
			this->MinimumSize = System::Drawing::Size(645, 574);
			this->Name = L"ExperimentUI";
			this->Text = L"SensoHapt";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->subjectNumber))->EndInit();
			this->statusStrip1->ResumeLayout(false);
			this->statusStrip1->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

		void InitializeProgrammerComponents()
		{
			openFileDialog = gcnew OpenFileDialog{};
			openFileDialog->InitialDirectory = Application::StartupPath;
			openFileDialog->Filter = "XML files (*.xml)|*.xml";
			openFileDialog->FilterIndex = 1;
			openFileDialog->RestoreDirectory = false;

			configXmlPath = gcnew System::String(Application::StartupPath + L"\\UserSettings\\experimentXmlPath.xml");

			experimentStatusLabel->Text = L"Please load a valid experiment";
			statusLabel->Text = L"Status: No experiment loaded";

			std::wstring configXmlPathStd = context.marshal_as<std::wstring>(configXmlPath);

			xml_document doc;

			if (doc.load_file(configXmlPathStd.c_str()))
			{
				std::string s{ doc.child("ExperimentConfigPath").attribute("path").as_string() };
				if (!(s.empty()))
				{
					loadExperimentConfig(s);
				}
			}
			else
			{
				File::Create(configXmlPath)->Close();
				std::string startupPath = context.marshal_as<std::string>(Application::StartupPath);
				doc.reset();
				doc.append_child("ExperimentConfigPath").append_attribute("path");
				std::string defaultTrialDataPath = startupPath + "\\TrialData";
				doc.append_child("TrialDataPath").append_attribute("path").set_value(defaultTrialDataPath.c_str());
				std::string defaultExperimentDetailPath = startupPath + "\\ExperimentDetails";
				doc.append_child("ExperimentDetailPath").append_attribute("path").set_value(defaultExperimentDetailPath.c_str());
				std::string defaultAnswersPath = startupPath + "\\Answers";
				doc.append_child("AnswersPath").append_attribute("path").set_value(defaultAnswersPath.c_str());
				std::string defaultHapticDeviceName = DEFAULT_HD_NAME;
				doc.append_child("HapticDeviceName").append_attribute("name").set_value(defaultHapticDeviceName.c_str());
				std::string defaultDaqName = DEFAULT_DAQ_NAME;
				doc.append_child("DaqName").append_attribute("name").set_value(defaultDaqName.c_str());
				std::string defaultLeftSensorCalPath = startupPath + "\\SensorCalibrations\\FT8568.cal";
				doc.append_child("LeftSensorCalibration").append_attribute("path").set_value(defaultLeftSensorCalPath.c_str());
				std::string defaultRightSensorCalPath = startupPath + "\\SensorCalibrations\\FT8569.cal";
				doc.append_child("RightSensorCalibration").append_attribute("path").set_value(defaultRightSensorCalPath.c_str());
				doc.save_file(configXmlPathStd.c_str());
			}
		}

		void ConvertXmlNodeToTreeNode(const xml_node& xmlNode, TreeNodeCollection^ treeNodes)
		{
			System::String^ nodeName = gcnew System::String(xmlNode.name());
			TreeNode^ newTreeNode = treeNodes->Add(nodeName);

			newTreeNode->Text = "<" + nodeName + ">";
			for (const auto& attribute : xmlNode.attributes())
			{
				System::String^ attributeName = gcnew System::String(attribute.name());
				TreeNode^ attributeNode = newTreeNode->Nodes->Add(attributeName);
				attributeNode->Text = attributeName + L" : " + gcnew System::String(attribute.as_string());
			}
			for (const auto& childNode : xmlNode.children()) ConvertXmlNodeToTreeNode(childNode, newTreeNode->Nodes);
		}

		void loadExperimentConfig(const std::string& filePath)
		{
			System::String^ extension = Path::GetExtension(gcnew System::String(filePath.c_str()));
			if (extension->Equals(L".xml"))
			{
				try
				{
					//Try to parse the xml
					ExperimentCommon::expInfo = std::make_shared<ExperimentInfo>( ExperimentParserXML{}(filePath) );
				
					//Update UI
					statusLabel->Text = L"Status: Experiment loaded";
					statusLabel->ForeColor = ForeColor.LightSeaGreen;
					experimentStatusLabel->Text = L"Ready to run";
					runButton->Enabled = true;

					//Update tree view (xml visualization)
					treeXml->Nodes->Clear();
					xml_document doc;
					doc.load_file(filePath.c_str());
					ConvertXmlNodeToTreeNode(doc.first_child(), treeXml->Nodes);
					treeXml->Nodes[0]->Expand();

					//Update xml configuration file
					doc.reset();
					std::wstring configXmlPathStd = context.marshal_as<std::wstring>(configXmlPath);
					doc.load_file(configXmlPathStd.c_str());
					xml_node node = doc.child("ExperimentConfigPath");
					if (node)
						node.attribute("path").set_value(filePath.c_str());
					else
					{
						doc.append_child("ExperimentConfigPath");
						doc.child("ExperimentConfigPath").append_attribute("path");
						doc.child("ExperimentConfigPath").attribute("path").set_value(filePath.c_str());
					}
					doc.save_file(configXmlPathStd.c_str());
				}
				catch (ExperimentParser::ParseException e)
				{
					statusLabel->Text = L"Status:\n" + gcnew System::String(e.description().c_str());
					statusLabel->ForeColor = ForeColor.IndianRed;
					experimentStatusLabel->Text = L"Failed to load experiment";
				}

			}
		}

		System::Void runButton_Click(System::Object^  sender, System::EventArgs^  e) 
		{
		
			System::Decimal^ subject_number_sys = subjectNumber->Value;

			uint16_t* subject_number = new uint16_t{ System::Decimal::ToUInt16(*subject_number_sys) };
#ifdef LOG_DEBUG
			Debug::WriteLine(*subject_number);
#endif

			DWORD experiment_thread_ID;
			experiment_thread_handle = CreateThread(
				NULL,
				0,
				experimentMain,
				(void*)subject_number,
				0,
				&experiment_thread_ID);

			if (experiment_thread_handle == NULL) ErrorExit(L"Failed to create Experiment thread");
#ifdef LOG_DEBUG
			else Debug::WriteLine(L"Created experiment thread with ID: " + experiment_thread_ID.ToString(), L"Thread Success");
#endif

			auto k = this->Controls;
			for each (System::Windows::Forms::Control^ c in k) c->Enabled = false;
			this->Hide();
			WaitForSingleObject(experiment_thread_handle, INFINITE);
			Application::Restart();
		}

		System::Void settingsButton_Click(System::Object^  sender, System::EventArgs^  e) 
		{ 
			SettingsUI settingsForm;
			settingsForm.ShowDialog();
		}
		
		System::Void loadExperimentButton_Click(System::Object^  sender, System::EventArgs^  e) 
		{
			//Launch browser with supported extensions
			if (openFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
			{
				std::string fileNameStd = context.marshal_as<std::string>(openFileDialog->FileName);
#ifdef LOG_DEBUG
				Debug::WriteLine(gcnew System::String(fileNameStd.c_str()));
#endif
				loadExperimentConfig(fileNameStd);
			}
		}
	};
}
