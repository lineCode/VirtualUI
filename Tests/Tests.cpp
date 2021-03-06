﻿// Tests.cpp: определяет точку входа для приложения.
//

#include <Miscellaneous/Time.h>
#include <Processes/Process.h>
#include <Processes/Threading.h>
#include <Miscellaneous/DynamicString.h>
#include <UserInterface/ShapeBase.h>
#include <UserInterface/Templates.h>
#include <UserInterface/ControlBase.h>
#include <Streaming.h>
#include <PlatformDependent/Direct2D.h>
#include <PlatformDependent/Direct3D.h>
#include <Miscellaneous/Dictionary.h>
#include <UserInterface/ControlClasses.h>
#include <UserInterface/BinaryLoader.h>
#include <UserInterface/StaticControls.h>
#include <UserInterface/ButtonControls.h>
#include <UserInterface/GroupControls.h>
#include <UserInterface/Menues.h>
#include <UserInterface/OverlappedWindows.h>
#include <UserInterface/EditControls.h>
#include <Syntax/Tokenization.h>
#include <Syntax/Grammar.h>
#include <Syntax/MathExpression.h>
#include <Syntax/Regular.h>
#include <PlatformDependent/KeyCodes.h>
#include <PlatformDependent/NativeStation.h>
#include <ImageCodec/IconCodec.h>
#include <Processes/Shell.h>
#include <Network/HTTP.h>
#include <Network/Socket.h>
#include <Network/Punycode.h>
#include <Storage/Registry.h>
#include <Storage/TextRegistry.h>
#include <Storage/TextRegistryGrammar.h>
#include <Storage/Compression.h>
#include <Storage/Chain.h>
#include <Miscellaneous/ThreadPool.h>
#include <Storage/Archive.h>
#include <Storage/ImageVolume.h>
#include <PlatformDependent/Assembly.h>

#include "stdafx.h"
#include "Tests.h"

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#include <PlatformDependent/WindowStation.h>

#undef CreateWindow
#undef GetCurrentDirectory
#undef SetCurrentDirectory
#undef GetCommandLine
#undef CreateProcess
#undef GetCurrentTime
#undef CreateFile
#undef CreateSemaphore

using namespace Engine;
using namespace Engine::UI;

#define MAX_LOADSTRING 100

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
HWND Window;

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

Engine::Direct2D::D2DRenderDevice * Device;
Engine::UI::FrameShape * Shape;
Engine::UI::ITexture * Texture;

Engine::UI::HandleWindowStation * station = 0;

ID2D1DeviceContext * Target = 0;
IDXGISwapChain1 * SwapChain = 0;

SafePointer<Engine::Streaming::TextWriter> conout;
SafePointer<Engine::UI::InterfaceTemplate> Template;

UI::IInversionEffectRenderingInfo * Inversion = 0;

SafePointer<Menues::Menu> menu;

ENGINE_PACKED_STRUCTURE(TestPacked)
	uint8 foo;
	uint32 bar;
	uint16 foobar;
ENGINE_END_PACKED_STRUCTURE

void CreateBlangSpelling(Syntax::Spelling & spelling)
{
	spelling.BooleanFalseLiteral = L"false";
	spelling.BooleanTrueLiteral = L"true";
	spelling.CommentEndOfLineWord = L"//";
	spelling.CommentBlockOpeningWord = L"/*";
	spelling.CommentBlockClosingWord = L"*/";
	spelling.IsolatedChars << L'(';
	spelling.IsolatedChars << L')';
	spelling.IsolatedChars << L'[';
	spelling.IsolatedChars << L']';
	spelling.IsolatedChars << L'{';
	spelling.IsolatedChars << L'}';
	spelling.IsolatedChars << L',';
	spelling.IsolatedChars << L';';
	spelling.IsolatedChars << L'^';
	spelling.IsolatedChars << L'.';
	spelling.IsolatedChars << L'~';
	spelling.IsolatedChars << L'@';
	spelling.ContinuousCharCombos << L"#";
	spelling.ContinuousCharCombos << L"=";
	spelling.ContinuousCharCombos << L"+";
	spelling.ContinuousCharCombos << L"-";
	spelling.ContinuousCharCombos << L"*";
	spelling.ContinuousCharCombos << L"/";
	spelling.ContinuousCharCombos << L"%";
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	CoInitializeEx(0, COINIT::COINIT_APARTMENTTHREADED);
	SetProcessDPIAware();

	AllocConsole();
	SetConsoleTitleW(L"ui tests");
	SafePointer<Engine::Streaming::FileStream> constream = new Engine::Streaming::FileStream(Engine::IO::GetStandartOutput());
	conout.SetReference(new Engine::Streaming::TextWriter(constream));

	(*conout) << IO::GetCurrentDirectory() << IO::NewLineChar;
	(*conout) << L"Full path      : " << IO::GetExecutablePath() << IO::NewLineChar;
	(*conout) << L"Directory      : " << IO::Path::GetDirectory(IO::GetExecutablePath()) << IO::NewLineChar;
	(*conout) << L"File name      : " << IO::Path::GetFileName(IO::GetExecutablePath()) << IO::NewLineChar;
	(*conout) << L"Clear file name: " << IO::Path::GetFileNameWithoutExtension(IO::GetExecutablePath()) << IO::NewLineChar;
	(*conout) << L"Extension      : " << IO::Path::GetExtension(IO::GetExecutablePath()) << IO::NewLineChar;
	(*conout) << L"Scale          : " << UI::Windows::GetScreenScale() << IO::NewLineChar;
	(*conout) << L"Locale         : " << Assembly::GetCurrentUserLocale() << IO::NewLineChar;

	IO::SetCurrentDirectory(IO::Path::GetDirectory(IO::GetExecutablePath()));

	/*{
		SafePointer<Tasks::ThreadPool> pool = new Tasks::ThreadPool;
		SafePointer<Streaming::Stream> output = new Streaming::FileStream(L"arch.ecsa", Streaming::AccessReadWrite, Streaming::CreateAlways);
		SafePointer<Storage::NewArchive> arch = Storage::CreateArchive(output, 2);
		SafePointer<Streaming::Stream> f1 = new Streaming::FileStream(L"EngineRuntime_x86.lib", Streaming::AccessRead, Streaming::OpenExisting);
		SafePointer<Streaming::Stream> f2 = new Streaming::FileStream(L"test.eui", Streaming::AccessRead, Streaming::OpenExisting);
		arch->SetFileName(1, L"EngineRuntime_x86.lib");
		arch->SetFileType(1, L"LIB");
		arch->SetFileID(1, 666);
		arch->SetFileCustom(1, 6666);
		arch->SetFileName(2, L"test.eui");
		arch->SetFileType(2, L"EUI");
		arch->SetFileID(2, 77);
		arch->SetFileCustom(2, 7777);
		arch->SetFileCreationTime(2, Time(2018, 6, 28, 12, 0, 0, 0));
		arch->SetFileAttribute(2, L"kornevgen", L"pidor");
		arch->SetFileData(1, f1, Storage::MethodChain(Storage::CompressionMethod::LempelZivWelch, Storage::CompressionMethod::Huffman), Storage::CompressionQuality::Sequential, pool);
		arch->SetFileData(2, f2, Storage::MethodChain(Storage::CompressionMethod::LempelZivWelch, Storage::CompressionMethod::Huffman), Storage::CompressionQuality::Sequential, pool);

		arch->Finalize();
	}*/
	/*{
		SafePointer<Streaming::Stream> source = new Streaming::FileStream(L"arch.ecsa", Streaming::AccessRead, Streaming::OpenExisting);
		SafePointer<Storage::Archive> arc = Storage::OpenArchive(source);

		int t = 666;
	}*/

	SafePointer<IResourceLoader> resource_loader = Engine::NativeWindows::CreateCompatibleResourceLoader();

    // TODO: разместите код здесь.

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TESTS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

	// Starting D3D
	Direct3D::CreateDevices();
	Direct3D::CreateD2DDeviceContextForWindow(::Window, &Target, &SwapChain);
	Device = new Engine::Direct2D::D2DRenderDevice(Target);
	{
		{
			UI::Zoom = 2.0;
			::Template.SetReference(new Engine::UI::InterfaceTemplate());
			{
				//SafePointer<Streaming::Stream> Source = new Streaming::FileStream(L"Test.eui", Streaming::AccessRead, Streaming::OpenExisting);
				SafePointer<Streaming::Stream> Source = Assembly::QueryResource(L"GUI");
				Engine::UI::Loader::LoadUserInterfaceFromBinary(*::Template, Source, resource_loader, 0);
			}
			station = new HandleWindowStation(::Window);
			station->SetRenderingDevice(Device);
			auto Main = station->GetDesktop();
			SendMessageW(::Window, WM_SIZE, 0, 0);
			SafePointer<Template::FrameShape> back = new Template::FrameShape;
			{
				SafePointer<Template::TextureShape> Back = new Template::TextureShape;
				Back->From = Rectangle::Entire();
				Back->RenderMode = TextureShape::TextureRenderMode::Fit;
				Back->Position = Rectangle::Entire();
				Back->Texture = ::Template->Texture[L"Wallpaper"];
				SafePointer<Template::BarShape> Fill = new Template::BarShape;
				Fill->Gradient << GradientPoint(0xFF303050);
				back->Children.Append(Back);
				back->Children.Append(Fill);
			}
			Main->As<TopLevelWindow>()->Background.SetRetain(back);

			station->GetVisualStyles().MenuArrow.SetReference(::Template->Application[L"a"]);
			SafePointer<Template::FrameShape> MenuBack = new Template::FrameShape;
			MenuBack->Position = UI::Rectangle::Entire();
			{
				SafePointer<Template::BlurEffectShape> Blur = new Template::BlurEffectShape;
				Blur->Position = UI::Rectangle::Entire();
				Blur->BlurPower = 20.0;
				SafePointer<Template::BarShape> Bk = new Template::BarShape;
				Bk->Position = UI::Rectangle::Entire();
				Bk->Gradient << GradientPoint(Color(64, 64, 64, 128));
				SafePointer<Template::BarShape> Left = new Template::BarShape;
				Left->Position = UI::Rectangle(0, 0, UI::Coordinate(0, 1.0, 0.0), UI::Coordinate::Bottom());
				Left->Gradient << GradientPoint(Color(255, 255, 255, 255));
				SafePointer<Template::BarShape> Top = new Template::BarShape;
				Top->Position = UI::Rectangle(0, 0, UI::Coordinate::Right(), UI::Coordinate(0, 1.0, 0.0));
				Top->Gradient << GradientPoint(Color(255, 255, 255, 255));
				SafePointer<Template::BarShape> Right = new Template::BarShape;
				Right->Position = UI::Rectangle(UI::Coordinate::Right() - UI::Coordinate(0, 1.0, 0.0), 0, UI::Coordinate::Right(), UI::Coordinate::Bottom());
				Right->Gradient << GradientPoint(Color(255, 255, 255, 255));
				SafePointer<Template::BarShape> Bottom = new Template::BarShape;
				Bottom->Position = UI::Rectangle(0, UI::Coordinate::Bottom() - UI::Coordinate(0, 1.0, 0.0), UI::Coordinate::Right(), UI::Coordinate::Bottom());
				Bottom->Gradient << GradientPoint(Color(255, 255, 255, 255));
				MenuBack->Children.Append(Left);
				MenuBack->Children.Append(Top);
				MenuBack->Children.Append(Right);
				MenuBack->Children.Append(Bottom);
				MenuBack->Children.Append(Bk);
				MenuBack->Children.Append(Blur);
			}
			station->GetVisualStyles().MenuBackground.SetRetain(MenuBack);
			station->GetVisualStyles().MenuBorder = int(UI::Zoom * 4.0);
			station->GetVisualStyles().WindowCloseButton.SetRetain(::Template->Dialog[L"Header"]->Children[0].Children.ElementAt(0));
			station->GetVisualStyles().WindowMaximizeButton.SetRetain(::Template->Dialog[L"Header"]->Children[0].Children.ElementAt(1));
			station->GetVisualStyles().WindowMinimizeButton.SetRetain(::Template->Dialog[L"Header"]->Children[0].Children.ElementAt(2));
			station->GetVisualStyles().WindowHelpButton.SetRetain(::Template->Dialog[L"Header"]->Children[0].Children.ElementAt(3));
			station->GetVisualStyles().WindowFixedBorder = 10;
			station->GetVisualStyles().WindowSizableBorder = 15;
			station->GetVisualStyles().WindowCaptionHeight = 60;
			station->GetVisualStyles().WindowSmallCaptionHeight = 40;
			{
				SafePointer<Template::BlurEffectShape> Blur = new Template::BlurEffectShape;
				Blur->Position = UI::Rectangle::Entire();
				Blur->BlurPower = 20.0;
				SafePointer<Template::BarShape> BkActive = new Template::BarShape;
				BkActive->Position = UI::Rectangle::Entire();
				BkActive->Gradient << GradientPoint(Color(96, 96, 96, 128));
				SafePointer<Template::BarShape> BkInactive = new Template::BarShape;
				BkInactive->Position = UI::Rectangle::Entire();
				BkInactive->Gradient << GradientPoint(Color(16, 16, 16, 128));
				SafePointer<Template::FrameShape> Decor = new Template::FrameShape;
				Decor->Position = UI::Rectangle::Entire();
				SafePointer<Template::TextShape> Title = new Template::TextShape;
				Title->Position = Template::Rectangle(
					Template::Coordinate(Template::IntegerTemplate::Undefined(L"Border"), 20.0, 0.0),
					Template::Coordinate(Template::IntegerTemplate::Undefined(L"Border"), 0.0, 0.0),
					Template::Coordinate(Template::IntegerTemplate::Undefined(L"= 0 - Border - ButtonsWidth"), 0.0, 1.0),
					Template::Coordinate(Template::IntegerTemplate::Undefined(L"= Border + Caption"), 0.0, 0.0)
				);
				Title->Font = ::Template->Font[L"NormalFont"];
				Title->Text = Template::StringTemplate::Undefined(L"Text");
				Title->TextColor = UI::Color(0xFFFFFFFF);
				Title->HorizontalAlign = TextShape::TextHorizontalAlign::Left;
				Title->VerticalAlign = TextShape::TextVerticalAlign::Center;
				Decor->Children.Append(Title);

				SafePointer<Template::FrameShape> Active = new Template::FrameShape;
				Active->Position = Rectangle::Entire();
				Active->Children.Append(Decor);
				Active->Children.Append(BkActive);
				Active->Children.Append(Blur);
				SafePointer<Template::FrameShape> Inactive = new Template::FrameShape;
				Inactive->Position = Rectangle::Entire();
				Inactive->Children.Append(Decor);
				Inactive->Children.Append(BkInactive);
				Inactive->Children.Append(Blur);
				station->GetVisualStyles().WindowActiveView.SetRetain(Active);
				station->GetVisualStyles().WindowInactiveView.SetRetain(Inactive);
				station->GetVisualStyles().CaretWidth = 2;
			}
			SafePointer<Template::FrameShape> wb = new Template::FrameShape;
			wb->Children.Append(::Template->Application[L"Waffle"]);
			wb->Opacity = 0.5;
			wb->RenderMode = FrameShape::FrameRenderMode::Layering;
			station->GetVisualStyles().WindowDefaultBackground.SetRetain(wb);

			menu = new Menues::Menu(::Template->Dialog[L"Menu"]);

			class _cb : public Windows::IWindowEventCallback
			{
			public:
				virtual void OnInitialized(UI::Window * window) override
				{
					(*conout) << L"Callback: Initialized, window = " << string(static_cast<handle>(window)) << IO::NewLineChar;
				}
				virtual void OnControlEvent(UI::Window * window, int ID, Window::Event event, UI::Window * sender) override
				{
					(*conout) << L"Callback: Event with ID = " << ID << L", window = " << string(static_cast<handle>(window)) << L", sender = " << string(static_cast<handle>(sender)) << IO::NewLineChar;
					if (event == Window::Event::Command || event == Window::Event::MenuCommand) {
						if (ID == 876) {
							menu->RunPopup(sender, station->GetCursorPos());
						} else if (ID == 2) {
							auto bar = static_cast<Controls::ProgressBar *>(window->FindChild(888));
							bar->SetValue(min(max(bar->GetValue() + 0.05, 0.0), 1.0));
							window->FindChild(1)->Enable(true);
							if (bar->GetValue() == 1.0) sender->Enable(false);
						} else if (ID == 1) {
							auto bar = static_cast<Controls::ProgressBar *>(window->FindChild(888));
							bar->SetValue(min(max(bar->GetValue() - 0.05, 0.0), 1.0));
							window->FindChild(2)->Enable(true);
							if (bar->GetValue() == 0.0) sender->Enable(false);
						} else if (ID == 202) {
							static_cast<Controls::ToolButtonPart *>(window->FindChild(202))->Checked ^= true;
							static_cast<Controls::ToolButtonPart *>(window->FindChild(201))->Disabled ^= true;
						} else if (ID == 201) {
							auto bar = window->FindChild(888);
							if (bar->IsVisible()) {
								bar->HideAnimated(Animation::SlideSide::Top, 500, Animation::AnimationClass::Smooth);
							} else {
								bar->ShowAnimated(Animation::SlideSide::Left, 500, Animation::AnimationClass::Smooth);
							}
						}
					}
				}
				virtual void OnFrameEvent(UI::Window * window, Windows::FrameEvent event) override
				{
					(*conout) << L"Callback: ";
					if (event == Windows::FrameEvent::Move) (*conout) << L"Move";
					else if (event == Windows::FrameEvent::Close) (*conout) << L"Close";
					else if (event == Windows::FrameEvent::Minimize) (*conout) << L"Minimize";
					else if (event == Windows::FrameEvent::Maximize) (*conout) << L"Maximize";
					else if (event == Windows::FrameEvent::Help) (*conout) << L"Help";
					else if (event == Windows::FrameEvent::PopupMenuCancelled) (*conout) << L"Popup menu cancelled";
					(*conout) << L", window = " << string(static_cast<handle>(window)) << IO::NewLineChar;
				}
			};
			auto Callback = new _cb;
			class _cb2 : public Windows::IWindowEventCallback
			{
			public:
				virtual void OnInitialized(UI::Window * window) override {}
				virtual void OnControlEvent(UI::Window * window, int ID, Window::Event event, UI::Window * sender) override
				{
					if (ID == 1) {
						auto group1 = window->FindChild(101);
						auto group2 = window->FindChild(102);
						if (group1->IsVisible()) {
							group1->HideAnimated(Animation::SlideSide::Left, 500,
								Animation::AnimationClass::Smooth, Animation::AnimationClass::Smooth);
							group2->ShowAnimated(Animation::SlideSide::Right, 500,
								Animation::AnimationClass::Smooth, Animation::AnimationClass::Smooth);
						} else {
							group2->HideAnimated(Animation::SlideSide::Left, 500,
								Animation::AnimationClass::Smooth, Animation::AnimationClass::Smooth);
							group1->ShowAnimated(Animation::SlideSide::Right, 500,
								Animation::AnimationClass::Smooth, Animation::AnimationClass::Smooth);
						}
					} else if (ID == 2) {
						auto group1 = window->FindChild(101);
						auto group2 = window->FindChild(102);
						if (group1->IsVisible()) {
							group1->HideAnimated(Animation::SlideSide::Right, 500,
								Animation::AnimationClass::Smooth, Animation::AnimationClass::Smooth);
							group2->ShowAnimated(Animation::SlideSide::Left, 500,
								Animation::AnimationClass::Smooth, Animation::AnimationClass::Smooth);
						} else {
							group2->HideAnimated(Animation::SlideSide::Right, 500,
								Animation::AnimationClass::Smooth, Animation::AnimationClass::Smooth);
							group1->ShowAnimated(Animation::SlideSide::Left, 500,
								Animation::AnimationClass::Smooth, Animation::AnimationClass::Smooth);
						}
					}
				}
				virtual void OnFrameEvent(UI::Window * window, Windows::FrameEvent event) override
				{
					if (event == Windows::FrameEvent::Close) window->Destroy();
				}
			};
			class _thl : public Controls::Edit::IEditHook
			{
			public:
				virtual string Filter(Controls::Edit * sender, const string & input) override
				{
					return input.Replace(L'0', L"ЫЫЫ");
				}
				virtual Array<uint8> * ColorHighlight(Controls::Edit * sender, const Array<uint32> & text) override
				{
					auto result = new Array<uint8>(text.Length());
					for (int i = 0; i < text.Length(); i++) {
						if (text[i] > 0xFFFF) result->Append(1);
						else result->Append(0);
					}
					return result;
				}
				virtual Array<UI::Color> * GetPalette(Controls::Edit * sender) override
				{
					auto result = new Array<UI::Color>(10);
					result->Append(Color(255, 0, 0));
					return result;
				}
			};
			class _thl2 : public Controls::MultiLineEdit::IMultiLineEditHook
			{
			public:
				virtual string Filter(Controls::MultiLineEdit * sender, const string & input, Point at) override
				{
					return input.Replace(L'0', L"ЫЫЫ");
				}
				virtual Array<uint8> * ColorHighlight(Controls::MultiLineEdit * sender, const Array<uint32> & text, int line) override
				{
					auto result = new Array<uint8>(text.Length());
					for (int i = 0; i < text.Length(); i++) {
						if (text[i] > 0xFFFF) result->Append(1);
						else if (text[i] >= L'0' && text[i] <= L'9') result->Append(2);
						else result->Append(0);
					}
					return result;
				}
				virtual Array<UI::Color> * GetPalette(Controls::MultiLineEdit * sender) override
				{
					auto result = new Array<UI::Color>(10);
					result->Append(Color(255, 0, 0));
					result->Append(Color(0, 0, 255));
					return result;
				}
			};
			auto Callback2 = new _cb2;
			auto Hook = new _thl;
			auto Hook2 = new _thl2;

			auto w = Windows::CreateFramedDialog(::Template->Dialog[L"Test2"], 0, UI::Rectangle::Invalid(), station);
			auto w2 = Windows::CreateFramedDialog(::Template->Dialog[L"Test"], Callback, UI::Rectangle(0, 0, Coordinate(0, 0.0, 0.7), Coordinate(0, 0.0, 0.55)), station);
			auto w3 = Windows::CreateFramedDialog(::Template->Dialog[L"Test3"], Callback2, UI::Rectangle::Invalid(), station);
			auto w4 = Windows::CreateFramedDialog(::Template->Dialog[L"Test3"], Callback2, UI::Rectangle::Invalid(), 0);
			auto w5 = Windows::CreateFramedDialog(::Template->Dialog[L"Test2"], 0, UI::Rectangle::Invalid(), 0);
			w2->FindChild(7777)->As<Controls::ColorView>()->SetColor(0xDDFF8040);
			w2->SetText(L"window");

			w->AddDialogStandartAccelerators();
			w2->AddDialogStandartAccelerators();
			w5->AddDialogStandartAccelerators();

			w->FindChild(101010)->As<Controls::Edit>()->SetHook(Hook);
			w5->FindChild(101010)->As<Controls::Edit>()->SetHook(Hook);
			w3->FindChild(212121)->As<Controls::MultiLineEdit>()->SetHook(Hook2);
			w4->FindChild(212121)->As<Controls::MultiLineEdit>()->SetHook(Hook2);

			w->Show(true);
			w2->Show(true);
			w3->Show(true);
			w4->Show(true);
			w5->Show(true);

			(*conout) << L"Done!" << IO::NewLineChar;
		}
	}

	NativeWindows::RunMainMessageLoop();
    
    return 0;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TESTS));
    wcex.hCursor        = 0;
    wcex.hbrBackground  = 0;
    wcex.lpszMenuName   = 0;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить дескриптор экземпляра в глобальной переменной

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   ::Window = hWnd;

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Разобрать выбор в меню:
            switch (wmId)
            {
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_ACTIVATE:
		SetTimer(hWnd, 1, 25, 0);
		break;
	case WM_TIMER:
		InvalidateRect(hWnd, 0, FALSE);
		if (station) return station->ProcessWindowEvents(message, wParam, lParam);
		return DefWindowProc(hWnd, message, wParam, lParam);
    case WM_PAINT:
        {
			RECT Rect;
			GetClientRect(hWnd, &Rect);
            HDC hdc = GetDC(hWnd);
			//FillRect(hdc, &Rect, (HBRUSH) GetStockObject(LTGRAY_BRUSH));

			//ValidateRect(hWnd, 0);
			if (Target) {
				Target->SetDpi(96.0f, 96.0f);
				Target->BeginDraw();
				Device->SetTimerValue(GetTimerValue());
				if (station) station->Render();
				Target->EndDraw();
				SwapChain->Present(1, 0);
			}
			ValidateRect(hWnd, 0);
            ReleaseDC(hWnd, hdc);
        }
        break;
	case WM_SIZE:
	{
		RECT Rect;
		GetClientRect(hWnd, &Rect);
		Direct3D::ResizeRenderBufferForD2DDevice(Target, SwapChain);
		if (station) station->ProcessWindowEvents(message, wParam, lParam);
	}
		break;
    case WM_DESTROY:
		NativeWindows::ExitMainLoop();
        break;
    default:
		if (station) return station->ProcessWindowEvents(message, wParam, lParam);
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}