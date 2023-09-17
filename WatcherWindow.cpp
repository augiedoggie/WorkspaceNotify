// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2023 Chris Roberts


#include "WatcherWindow.h"

#include <Bitmap.h>
#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <ColorControl.h>
#include <File.h>
#include <FindDirectory.h>
#include <LayoutBuilder.h>
#include <Notification.h>
#include <Path.h>
#include <PopUpMenu.h>
#include <RadioButton.h>
#include <SeparatorView.h>
#include <Slider.h>
#include <Spinner.h>
#include <StringView.h>


const char* kAppTitle = "WorkspaceNotify";

const char* kKeyTimeout = "timeout";
const char* kKeyTitle = "title";
const char* kKeyFontSize = "font_size";
const char* kKeyAutoRun = "auto_run";
const char* kKeyForeground = "foreground";
const char* kKeyBackground = "background";

const float kDefaultTimeout = 1.5;
const float kDefaultFontSize = 40.0;
const bool kDefaultAutoRun = true;
const rgb_color kDefaultForeground = {0, 0, 0};
const rgb_color kDefaultBackground = {0, 185, 230};
const char* kDefaultTitle = "Workspace %workspace%";

enum {
	kActionBackground = 'BGND',
	kActionForeground = 'FGND',
	kActionColor = 'COLR',
	kActionDefaults = 'DFLT',
	kActionFont = 'FONT',
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

	fForegroundPreview = new BView("ForegroundColorPreview", B_WILL_DRAW);
	fForegroundPreview->SetExplicitMaxSize(BSize(15, 15));

	fBackgroundPreview = new BView("BackgroundColorPreview", B_WILL_DRAW);
	fBackgroundPreview->SetExplicitMaxSize(BSize(15, 15));

	fBackgroundButton = new BRadioButton("Background", new BMessage(kActionBackground));
	fBackgroundButton->SetValue(B_CONTROL_ON);

	fPreviewView = new BView("IconPreviewView", B_WILL_DRAW);
	fPreviewView->SetExplicitSize(BSize(31, 31));
	fPreviewView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	// clang-format off
	BGroupLayout* iconLayout = BLayoutBuilder::Group<>(B_HORIZONTAL)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.Add(fPreviewView)
		.Add(new BSeparatorView(B_VERTICAL))
		.AddGroup(B_VERTICAL)
			.AddGroup(B_HORIZONTAL)
				.Add(fFontSizeSpinner = new BSpinner("FontSpinner", "Font Size:", new BMessage(kActionFont)))
				.AddGlue(2.0)
			.End()
			// .Add(new BSeparatorView(B_HORIZONTAL))
			.AddGroup(B_HORIZONTAL)
				.Add(fForegroundPreview)
				.Add(fForegroundButton = new BRadioButton("Foreground", new BMessage(kActionForeground)))
				.Add(new BSeparatorView(B_VERTICAL))
				.Add(fBackgroundPreview)
				.Add(fBackgroundButton)
				.AddGlue()
			.End()
			.Add(fColorControl = new BColorControl(B_ORIGIN, B_CELLS_32x8, 8.0, "ColorControl", new BMessage(kActionColor), true))
		.End();

	BBox* iconBox = new BBox("IconBBox");
	iconBox->SetLabel("Notification Icon");
	iconBox->AddChild(iconLayout->View());

	BStringView* tipView = new BStringView("TitleTipStringView", "%workspace% will be replaced with the current workspace");
	BFont font;
	tipView->GetFont(&font);
	font.SetFace(B_ITALIC_FACE);
	tipView->SetFont(&font);
	tipView->SetAlignment(B_ALIGN_CENTER);

	BGroupLayout* textLayout = BLayoutBuilder::Group<>(B_VERTICAL)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.Add(fTitleControl = new BTextControl("", "Workspace %workspace%", NULL))
		.Add(tipView);

	BBox* textBox = new BBox("TextBox");
	textBox->SetLabel("Notification Title");
	textBox->AddChild(textLayout->View());

	fTimeoutSlider = new BSlider("TimeoutSlider", "seconds", new BMessage(kActionTimeout), 0, 60, B_HORIZONTAL);
	fTimeoutSlider->SetLimitLabels("0.0", "30.0");
	fTimeoutSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fTimeoutSlider->SetHashMarkCount(31);
	fTimeoutSlider->SetModificationMessage(new BMessage(kActionTimeout));

	tipView = new BStringView("TimeoutTipStringView", "Setting the timeout to 0.0 will use the system default timing");
	tipView->GetFont(&font);
	font.SetFace(B_ITALIC_FACE);
	tipView->SetFont(&font);
	tipView->SetAlignment(B_ALIGN_CENTER);

	BGroupLayout* timeoutLayout = BLayoutBuilder::Group<>(B_VERTICAL)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.Add(fTimeoutSlider)
		.Add(tipView);

	BBox* timeoutBox = new BBox("TimeoutBox");
	timeoutBox->SetLabel("Notification Timeout");
	timeoutBox->AddChild(timeoutLayout->View());

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_WINDOW_SPACING)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(textBox)
		.Add(iconBox)
		.Add(timeoutBox)
		.Add(fAutoRunCheckBox = new BCheckBox("Start WorkspaceNotify when Haiku boots"))
		.AddGlue(10.0)
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

	_UpdatePreview();
}


bool
WatcherWindow::QuitRequested()
{
	_SaveSettings();
	return BWindow::QuitRequested();
}


void
WatcherWindow::MessageReceived(BMessage* message)
{
#ifdef DEBUG
	std::cout << "WatcherWindow::MessageReceived()" << std::endl;
	message->PrintToStream();
#endif

	switch (message->what) {
		case kActionDefaults:
		{
			fTimeoutSlider->SetValue(kDefaultTimeout * 2);
			_UpdateSliderLabel();

			fFontSizeSpinner->SetValue(kDefaultFontSize);

			fTitleControl->SetText(kDefaultTitle);
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

			_UpdatePreview();

			_SaveSettings();
			break;
		}
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

			_UpdatePreview();
			break;
		case kActionFont:
			_UpdatePreview();
			break;
		case kActionTimeout:
			_UpdateSliderLabel();
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
	BString title(fTitleControl->Text());
	if (title.Length() > 0) {
		BString notString;
		notString << workspace + 1;
		title.ReplaceAll("%workspace%", notString);
		notification.SetTitle(title);
	}
	notification.SetMessageID(kAppTitle);

	BBitmap* bitmap = new BBitmap(BRect(0, 0, 31, 31), B_RGBA32, true);
	if (_RenderBitmap(workspace, *bitmap) == B_OK)
		notification.SetIcon(bitmap);

	notification.Send(fTimeoutSlider->Value() / 2.0 * 1000.0 * 1000.0);

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
	fTimeoutSlider->SetValue(floatValue * 2);
	_UpdateSliderLabel();

	floatValue = message.GetFloat(kKeyFontSize, kDefaultFontSize);
	fFontSizeSpinner->SetValue(floatValue);

	fTitleControl->SetText(message.GetString(kKeyTitle, kDefaultTitle));

	bool bValue = message.GetBool(kKeyAutoRun, kDefaultAutoRun);
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

	message.AddFloat(kKeyTimeout, fTimeoutSlider->Value() / 2.0);

	message.AddFloat(kKeyFontSize, fFontSizeSpinner->Value());

	message.AddString(kKeyTitle, fTitleControl->Text());

	message.AddBool(kKeyAutoRun, fAutoRunCheckBox->Value());

	message.AddColor(kKeyForeground, fForegroundColor);

	message.AddColor(kKeyBackground, fBackgroundColor);

	return message.Flatten(&prefsFile);
}


void
WatcherWindow::_UpdateSliderLabel()
{
	BString label;
	label.SetToFormat("%.1f seconds", fTimeoutSlider->Value() / 2.0);
	fTimeoutSlider->SetLabel(label);
}


void
WatcherWindow::_UpdatePreview()
{
	BBitmap* bitmap = new BBitmap(BRect(0, 0, 31, 31), B_RGBA32, true);
	if (_RenderBitmap(2, *bitmap, fPreviewView) == B_OK)
		fPreviewView->SetViewBitmap(bitmap);

	delete bitmap;
}


status_t
WatcherWindow::_RenderBitmap(int32 workspace, BBitmap& bitmap, BView* parent)
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

	if (parent == NULL)
		// clear the view
		view->SetHighColor(B_TRANSPARENT_32_BIT);
	else
		view->SetHighColor(parent->ViewColor());

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
