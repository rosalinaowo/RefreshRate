#include <Windows.h>
#include <iostream>

using namespace std;

void PrintDDInfo(int& deviceIdx, DISPLAY_DEVICE& dd)
{
	wcout << L"Display device " << deviceIdx << L": " << dd.DeviceName << endl;
	wcout << L"  Device string: " << dd.DeviceString << endl;
	wcout << L"  State flags: " << dd.StateFlags << endl;
}

void PrintMonitorInfo(int& monitorIdx, DISPLAY_DEVICE& monitor)
{
	wcout << L"  Monitor " << monitorIdx << L": " << monitor.DeviceName << endl;
	wcout << L"    Device string: " << monitor.DeviceString << endl;
	wcout << L"    State flags: " << monitor.StateFlags << endl;
}

void PrintMonitorSettings(DEVMODEW &dm)
{
	wcout << L"    Resolution: " << dm.dmPelsWidth << L"x" << dm.dmPelsHeight << L"@" << dm.dmDisplayFrequency << endl;
	wcout << L"    Bits per pixel: " << dm.dmBitsPerPel << L"bpp " << endl;
	wcout << L"    Flags: " << dm.dmDisplayFlags << endl;
}

LONG ChangeSettings(DEVMODEW& dm, DWORD dwFlags)
{
	int changeResult = ChangeDisplaySettingsW(&dm, 0);
	switch (changeResult)
	{
	case DISP_CHANGE_SUCCESSFUL: wcout << L"Changed refresh rate to " << dm.dmDisplayFrequency << L"hz!"; break;
	case DISP_CHANGE_BADDUALVIEW: cerr << "The settings change was unsuccessful because the system is DualView capable."; break;
	case DISP_CHANGE_BADFLAGS: cerr << "An invalid set of flags was passed in."; break;
	case DISP_CHANGE_BADMODE: cerr << "The graphics mode is not supported."; break;
	case DISP_CHANGE_BADPARAM: cerr << "An invalid parameter was passed in. This can include an invalid flag or combination of flags."; break;
	case DISP_CHANGE_FAILED: cerr << "The display driver failed the specified graphics mode."; break;
	case DISP_CHANGE_NOTUPDATED: cerr << "Unable to write settings to the registry."; break;
	case DISP_CHANGE_RESTART: cerr << "The computer must be restarted for the graphics mode to work."; break;
	default: cerr << "Unknown error."; break;
	}

	cout << endl;
	return changeResult;
}

int main(int argc, char *argv[])
{
	DISPLAY_DEVICE dd;
	DEVMODEW dm;
	int deviceIdx = 0;
	bool shouldPrint = argc < 3;
	WCHAR* monitorDeviceName = new WCHAR[1];
	monitorDeviceName[0] = L'\0';
	int refreshRate = 0;

	if (argc == 3)
	{
		int wsLength = MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, NULL, 0);
		monitorDeviceName = new WCHAR[wsLength];
		if (wsLength == 0 || MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, monitorDeviceName, wsLength) == 0)
		{
			cerr << "Failed to convert monitor string to wide string." << endl;
			return 1;
		}

		refreshRate = strtol(argv[2], NULL, 10);
		if (refreshRate < 1)
		{
			cerr << "Invalid refresh rate specified.";
			return 1;
		}
	}

	while (true)
	{
		ZeroMemory(&dd, sizeof(dd));
		dd.cb = sizeof(dd);

		if (!EnumDisplayDevicesW(NULL, deviceIdx, &dd, 0)) { break; }
		if (shouldPrint) { PrintDDInfo(deviceIdx, dd); }

		DISPLAY_DEVICE monitor;
		int monitorIdx = 0;

		while (true)
		{
			ZeroMemory(&monitor, sizeof(monitor));
			monitor.cb = sizeof(monitor);
			ZeroMemory(&dm, sizeof(dm));
			dm.dmSize = sizeof(dm);

			if (!EnumDisplayDevicesW(dd.DeviceName, monitorIdx, &monitor, 0)) { break; }
			if (shouldPrint) { PrintMonitorInfo(monitorIdx, monitor); }

			if (!EnumDisplaySettingsW(dd.DeviceName, ENUM_CURRENT_SETTINGS, &dm)) { break; }
			if (shouldPrint) { PrintMonitorSettings(dm); }

			if (wcscmp(monitorDeviceName, monitor.DeviceName) == 0)
			{
				dm.dmDisplayFrequency = refreshRate;
				return ChangeSettings(dm, 0);
			}
			
			monitorIdx++;
		}

		deviceIdx++;
	}

	if (!shouldPrint)
	{
		wcout << L"Monitor " << monitorDeviceName << L" not found." << endl;
	}

	return 0;
}