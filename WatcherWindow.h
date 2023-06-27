// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2023 Chris Roberts


#include <Window.h>


class BCheckBox;
class BColorControl;
class BRadioButton;
class BSlider;
class BSpinner;
class BTextControl;


class WatcherWindow : public BWindow {
public:
					WatcherWindow(BRect frame);

	virtual void	MessageReceived(BMessage *message);

	virtual void	WorkspaceActivated(int32 workspace, bool state);

	virtual bool	QuitRequested();

private:
	status_t		_RenderBitmap(int32 workspace, BBitmap& bitmap, BView* parent = NULL);
	status_t		_LoadSettings();
	status_t		_SaveSettings();
	void			_UpdatePreview();
	void			_UpdateSliderLabel();

	BSlider*		fTimeoutSlider;
	BSpinner*		fFontSizeSpinner;
	BCheckBox*		fAutoRunCheckBox;
	BTextControl*	fTitleControl;
	BColorControl*	fColorControl;
	BRadioButton*	fForegroundButton;
	BRadioButton*	fBackgroundButton;
	BView*			fBackgroundPreview;
	BView*			fForegroundPreview;
	BView*			fPreviewView;
	rgb_color		fBackgroundColor;
	rgb_color		fForegroundColor;
};
