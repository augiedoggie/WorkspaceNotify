// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2023 Chris Roberts


#include "WatcherWindow.h"

#include <Bitmap.h>
#include <Button.h>
#include <CheckBox.h>
#include <ColorControl.h>
#include <File.h>
#include <FindDirectory.h>
#include <LayoutBuilder.h>
#include <MenuField.h>
#include <Notification.h>
#include <Path.h>
#include <PopUpMenu.h>
#include <RadioButton.h>
#include <SeparatorView.h>
#include <Spinner.h>
#include <StringView.h>


const char* kAppTitle = "WorkspaceNotify";

const char* kKeyTimeout = "timeout";
const char* kKeyShowText = "show_text";
const char* kKeyFontSize = "font_size";
const char* kKeyAutoRun = "auto_run";
const char* kKeyForeground = "foreground";
const char* kKeyBackground = "background";

const float kDefaultTimeout = 1.5;
const float kDefaultFontSize = 40.0;
const bool kDefaultShowText = true;
const bool kDefaultAutoRun = true;
const rgb_color kDefaultForeground = {0, 0, 0};
const rgb_color kDefaultBackground = {0, 185, 230};

enum {
	kActionBackground = 'BGND',
	kActionForeground = 'FGND',
	kActionColor = 'COLR',
	kActionDefaults = 'DFLT',
	kActionRun = 'RRUN',
	kActionTest = 'TEST',
	kActionTimeout = 'TMUT'
};


WatcherWindow::WatcherWindow(BRect frame)
	:
	BWindow(frame, "WorkspaceNotify", B_TITLED_WINDOW, B_NOT_CLOSABLE | B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS | B_QUIT_ON_WINDOW_CLOSE | B_ASYNCHRONOUS_CONTROLS, B_ALL_WORKSPACES),
	fBackgroundColor(kDefaultBackground),
	fForegroundColor(kDefaultForeground)
{
	BButton* runButton = new BButton("Hide Window", new BMessage(kActionRun));

	BPopUpMenu* timeoutMenu = new BPopUpMenu("TimeoutMenu");
	BLayoutBuilder::Menu<> builder = BLayoutBuilder::Menu<>(timeoutMenu);
	for (double x = 0; x <= 5; x+=0.5) {
		BString string;
		string.SetToFormat("%.1f", x);
		builder.AddItem(string, kActionTimeout);
	}

	fForegroundPreview = new BView("ForegroundColorPreview", B_WILL_DRAW);
	fForegroundPreview->SetExplicitMaxSize(BSize(16, 16));

	fBackgroundPreview = new BView("BackgroundColorPreview", B_WILL_DRAW);
	fBackgroundPreview->SetExplicitMaxSize(BSize(16, 16));

	fBackgroundButton = new BRadioButton("Background", new BMessage(kActionBackground));
	fBackgroundButton->SetValue(B_CONTROL_ON);

	// clang-format off
	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_WINDOW_SPACING)
		.SetInsets(B_USE_WINDOW_INSETS)
		.AddGroup(B_HORIZONTAL, 3.0)
			.Add(fTimeoutField = new BMenuField("Timeout:", timeoutMenu))
			.Add(new BStringView("SecondsView", "seconds"))
			.AddGlue(2.0)
		.End()
		.Add(fShowTextCheckBox = new BCheckBox("Show text", NULL))
		.AddGroup(B_HORIZONTAL)
			.Add(fFontSizeSpinner = new BSpinner("FontSpinner", "Font size:", NULL))
			.AddGlue(2.0)
		.End()
		.AddGroup(B_HORIZONTAL)
			.Add(fForegroundPreview)
			.Add(fForegroundButton = new BRadioButton("Foreground", new BMessage(kActionForeground)))
			.Add(new BSeparatorView(B_VERTICAL))
			.Add(fBackgroundPreview)
			.Add(fBackgroundButton)
			.AddGlue()
		.End()
		.Add(fColorControl = new BColorControl(B_ORIGIN, B_CELLS_32x8, 8.0, "ColorControl", new BMessage(kActionColor), true))
		.Add(fAutoRunCheckBox = new BCheckBox("Launch when Haiku boots"))
		.AddGroup(B_HORIZONTAL, B_USE_HALF_ITEM_SPACING)
			.Add(new BButton("Show Test", new BMessage(kActionTest)))
			.Add(new BButton("Defaults", new BMessage(kActionDefaults)))
			.AddGlue()
			.Add(new BButton("Quit", new BMessage(B_QUIT_REQUESTED)))
			.Add(runButton)
		.End()
	.End();
	// clang-format on

	fFontSizeSpinner->SetMinValue(10);
	fFontSizeSpinner->SetMaxValue(100);

	runButton->MakeDefault(true);

	_LoadSettings();
}


bool
WatcherWindow::QuitRequested() {
	_SaveSettings();
	return BWindow::QuitRequested();
}


void
WatcherWindow::MessageReceived(BMessage *message) {
	#ifdef DEBUG
	std::cout << "WatcherWindow::MessageReceived()" << std::endl;
	message->PrintToStream();
	#endif

	switch (message->what) {
		case kActionDefaults:
		{
			BMenu* menu = fTimeoutField->Menu();
			BString bufString;
			bufString.SetToFormat("%.1f", kDefaultTimeout);
			BMenuItem* item = menu->FindItem(bufString);
			if (item != NULL)
				item->SetMarked(true);

			fFontSizeSpinner->SetValue(kDefaultFontSize);

			fShowTextCheckBox->SetValue(kDefaultShowText);
			fAutoRunCheckBox->SetValue(kDefaultAutoRun);

			fBackgroundColor = kDefaultBackground;
			fBackgroundPreview->SetViewColor(fBackgroundColor);
			fBackgroundPreview->Invalidate();

			fForegroundColor = kDefaultForeground;
			fForegroundPreview->SetViewColor(fForegroundColor);
			fForegroundPreview->Invalidate();

			if (fBackgroundButton->Value() == B_CONTROL_ON)
				fColorControl->SetValue(fBackgroundColor);
			else
				fColorControl->SetValue(fForegroundColor);

			_SaveSettings();
		}
			break;
		case kActionTest:
			WorkspaceActivated(current_workspace(), true);
			break;
		case kActionForeground:
			fColorControl->SetValue(fForegroundColor);
			break;
		case kActionBackground:
			fColorControl->SetValue(fBackgroundColor);
			break;
		case kActionColor:
			if (fBackgroundButton->Value() == B_CONTROL_ON) {
				fBackgroundColor = fColorControl->ValueAsColor();
				fBackgroundPreview->SetViewColor(fBackgroundColor);
				fBackgroundPreview->Invalidate();
			} else {
				fForegroundColor = fColorControl->ValueAsColor();
				fForegroundPreview->SetViewColor(fForegroundColor);
				fForegroundPreview->Invalidate();
			}
			break;
		case kActionRun:
			if (Lock()) {
				Hide();
				Unlock();
			}
			_SaveSettings();
			break;
		default:
			BWindow::MessageReceived(message);
	}
}


void
WatcherWindow::WorkspaceActivated(int32 workspace, bool state)
{
	#ifdef DEBUG
	std::cout << "WorkspaceActivated: " << workspace << " " << state << std::endl;
	#endif

	if (!state)
		return;

	BNotification notification(B_INFORMATION_NOTIFICATION);
	if (fShowTextCheckBox->Value()) {
		BString notString("Workspace ");
		notString << workspace + 1;
		notification.SetTitle(notString);
	}
	notification.SetMessageID("WorkspaceNotify");

	BBitmap* bitmap = new BBitmap(BRect(0, 0, 31, 31), B_RGBA32, true);
	if (_RenderBitmap(workspace, *bitmap) == B_OK)
		notification.SetIcon(bitmap);

	float timeout = 0.0;
	BMenuItem* item = fTimeoutField->Menu()->FindMarked();
	if (item != NULL)
		notification.Send(atof(item->Label())*1000.0*1000.0);

	delete bitmap;
}


status_t
WatcherWindow::_LoadSettings()
{
	BPath prefsPath;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &prefsPath) != B_OK)
		return B_ERROR;

	prefsPath.Append(kAppTitle);
	BFile prefsFile;

	BMessage message;
	if (prefsFile.SetTo(prefsPath.Path(), B_READ_WRITE) == B_OK)
		message.Unflatten(&prefsFile);

	float floatValue = message.GetFloat(kKeyTimeout, kDefaultTimeout);
	BString timeoutString;
	timeoutString.SetToFormat("%.1f", floatValue);
	BMenu* menu = fTimeoutField->Menu();
	BMenuItem* item = menu->FindItem(timeoutString);
	if (item != NULL)
		item->SetMarked(true);

	floatValue = message.GetFloat(kKeyFontSize, kDefaultFontSize);
	fFontSizeSpinner->SetValue(floatValue);

	bool bValue = message.GetBool(kKeyShowText, kDefaultShowText);
	fShowTextCheckBox->SetValue(bValue);

	bValue = message.GetBool(kKeyAutoRun, kDefaultAutoRun);
	fAutoRunCheckBox->SetValue(bValue);

	fForegroundColor = message.GetColor(kKeyForeground, kDefaultForeground);
	fForegroundPreview->SetViewColor(fForegroundColor);

	fBackgroundColor = message.GetColor(kKeyBackground, kDefaultBackground);
	fBackgroundPreview->SetViewColor(fBackgroundColor);
	fColorControl->SetValue(fBackgroundColor);

	return B_OK;
}


status_t
WatcherWindow::_SaveSettings()
{
	BPath prefsPath;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &prefsPath) != B_OK)
		return B_ERROR;

	prefsPath.Append(kAppTitle);
	BFile prefsFile;

	if (prefsFile.SetTo(prefsPath.Path(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE) != B_OK)
		return B_ERROR;

	BMessage message;

	BMenuItem* item = fTimeoutField->Menu()->FindMarked();
	message.AddFloat(kKeyTimeout, atof(item->Label()));

	message.AddFloat(kKeyFontSize, fFontSizeSpinner->Value());

	message.AddBool(kKeyShowText, fShowTextCheckBox->Value());

	message.AddBool(kKeyAutoRun, fAutoRunCheckBox->Value());

	message.AddColor(kKeyForeground, fForegroundColor);

	message.AddColor(kKeyBackground, fBackgroundColor);

	return message.Flatten(&prefsFile);
}


status_t
WatcherWindow::_RenderBitmap(int32 workspace, BBitmap& bitmap)
{
	if (workspace > 31 || workspace < 0)
		return B_ERROR;

	BView* view = new BView(BRect(bitmap.Bounds()), "RenderView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_SUBPIXEL_PRECISE | B_TRANSPARENT_BACKGROUND);
	bitmap.AddChild(view);
	view->LockLooper();

	view->SetFont(be_bold_font);
	//FIXME need better font size calculation
	// double digit workspaces get decreased to 65%
	if (workspace < 9)
		view->SetFontSize(fFontSizeSpinner->Value());
	else
		view->SetFontSize(fFontSizeSpinner->Value() * 0.65);

	// clear the view
	view->SetHighColor(B_TRANSPARENT_32_BIT);
	view->FillRect(view->Bounds());

	view->SetHighColor(fBackgroundColor);
	view->FillRoundRect(view->Bounds(), 8, 8);

	// draw a slightly darker border
	view->SetHighColor(tint_color(view->HighColor(), B_DARKEN_1_TINT));
	view->StrokeRoundRect(view->Bounds(), 8, 8);

	view->SetHighColor(fForegroundColor);
	BString wString;
	wString.SetToFormat("%d", workspace + 1);
	font_height fontHeight;
	view->GetFontHeight(&fontHeight);
	view->MovePenTo(
		(view->Bounds().Width() / 2) - (view->StringWidth(wString) / 2) + (workspace < 9 ? 1 : 0),
		(view->Bounds().Height() / 2) + ((fontHeight.ascent - fontHeight.descent) / 2) - 1);
	view->DrawString(wString);

	view->Sync();
	view->UnlockLooper();
	bitmap.RemoveChild(view);
	delete view;

	return B_OK;
}
