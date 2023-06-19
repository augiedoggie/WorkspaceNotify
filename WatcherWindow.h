// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2023 Chris Roberts


#include <Window.h>


class BCheckBox;
class BMenuField;
class BSpinner;
class BTextControl;


class WatcherWindow : public BWindow {
public:
					WatcherWindow(BRect frame);

	virtual void	MessageReceived(BMessage *message);

	virtual void	WorkspaceActivated(int32 workspace, bool state);

	virtual bool	QuitRequested();

private:
	status_t		_RenderBitmap(int32 workspace, BBitmap& bitmap);
	status_t		_LoadSettings();
	status_t		_SaveSettings();

	BMenuField*		fTimeoutField;
	BSpinner*		fFontSizeSpinner;
	BCheckBox*		fShowTextCheckBox;
	BCheckBox*		fAutoRunCheckBox;
	rgb_color		fBackgroundColor;
	rgb_color		fForegroundColor;
};
