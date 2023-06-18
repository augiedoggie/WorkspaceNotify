// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2023 Chris Roberts

#include "WatcherWindow.h"

#include <Application.h>
#include <Alert.h>
#include <iostream>



class NotifyApp : public BApplication {
public:
	NotifyApp()
		:
		BApplication("application/x-vnd.cpr.WorkspaceNotify"),
		fAutoRun(false)
	{}

	virtual void
	MessageReceived(BMessage* message)
	{
		#ifdef DEBUG
		std::cout << "NotifyApp::MessageReceived()" << std::endl;
		message->PrintToStream();
		#endif

		switch (message->what) {
			case B_SILENT_RELAUNCH:
			{
				BWindow* window = WindowAt(0);
				if (window != NULL) {
					window->Lock();
					window->CenterOnScreen();
					window->Show();
					window->Unlock();
				}
			}
				break;
			default:
				BApplication::MessageReceived(message);
		}
	}

	virtual void
	ReadyToRun()
	{
		BRect frame;
		if (fAutoRun)
			frame.Set(-1000, -1000, -500, -500);
		else
			frame.Set(200, 200, 450, 450);

		WatcherWindow *window = new WatcherWindow(frame);
		window->Lock();

		if (!fAutoRun)
			window->CenterOnScreen();

		window->Show();

		if (fAutoRun)
			window->Hide();

		window->Unlock();
	}

	virtual void
	ArgvReceived(int32 argc, char** argv)
	{
		if (argc > 2) {
			std::cerr << "Error: Too many arguments" << std::endl;
			std::cout << "To run in background: " << argv[0] << " -r" << std::endl;
			exit(1);
		} else if (strcmp(argv[1], "-r") == 0)
			fAutoRun = true;
		else {
			std::cerr << "Error: Argument not understood" << std::endl;
			std::cout << "To run in background: " << argv[0] << " -r" << std::endl;
			exit(1);
		}
	}

	virtual void
	AboutRequested()
	{
		(new BAlert("AboutWindow", "WorkspaceNotify\nWritten by Chris Roberts", "OK", NULL, NULL, B_WIDTH_FROM_LABEL))->Go();
	}

private:
	bool		fAutoRun;
};


int
main(int /*argc*/, char** /*argv*/)
{
	NotifyApp app;
	app.Run();

	return 0;
}
