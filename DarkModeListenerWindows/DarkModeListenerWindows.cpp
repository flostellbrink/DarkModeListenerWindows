#include <windows.h>
#include <string>
#include <iostream>

constexpr DWORD format_size = 100;
constexpr DWORD dword_size = 4;

HKEY key;
HANDLE event;

static std::string format_error(const LSTATUS error)
{
	WCHAR buffer[format_size];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error, LANG_SYSTEM_DEFAULT, buffer, format_size, nullptr);
	std::wstring result(buffer);
	return std::string(result.begin(), result.end());
}

static void setup_listener_event()
{
	const auto path = L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize";

	// Open key
	const auto open_error = RegOpenKeyEx(HKEY_CURRENT_USER, path, 0, KEY_NOTIFY | KEY_READ, &key);
	if (open_error != ERROR_SUCCESS)
	{
		throw std::runtime_error("Failed to open key: " + format_error(open_error));
	}


	// Create event
	event = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	if (event == nullptr)
	{
		throw std::runtime_error("Failed to create event: " + std::to_string(GetLastError()));
	}
}

static void read_event()
{
	// Find flag based on os architecture
	BOOL is64_bit_os;
	IsWow64Process(GetCurrentProcess(), &is64_bit_os);
	auto flag = RRF_RT_ANY;
	if (is64_bit_os)
		flag |= KEY_WOW64_64KEY;
	else
		flag |= KEY_WOW64_32KEY;


	// Read registry key
	char data[dword_size];
	auto size = dword_size;
	const auto get_error = RegGetValueW(key, nullptr, L"AppsUseLightTheme", flag, nullptr, data, &size);
	if (get_error != ERROR_SUCCESS)
	{
		throw std::runtime_error("Failed to get value: " + format_error(get_error));
	}

	// Emit dark mode
	const auto light_theme = data[0] != 0 || data[1] != 0 || data[2] != 0 || data[3] != 0;
	std::cout << (light_theme ? "light" : "dark") << std::endl;

	// Register watcher
	ResetEvent(event);
	const auto watch_error = RegNotifyChangeKeyValue(key,
		TRUE,
		REG_NOTIFY_CHANGE_LAST_SET,
		event,
		TRUE);
	if (watch_error != ERROR_SUCCESS)
	{
		throw std::runtime_error("Failed to create event: " + format_error(watch_error));
	}

	// Wait for update
	if (WaitForSingleObject(event, INFINITE) == WAIT_FAILED)
	{
		throw std::runtime_error("Failed to wait for event: " + std::to_string(GetLastError()));
	}
}

int main()
{
	try {
		setup_listener_event();
		while (true)
		{
			read_event();
		}
	}
	catch (std::runtime_error & error)
	{
		std::cerr << error.what() << std::endl;
		RegCloseKey(key);
		CloseHandle(event);
		return 1;
	}
}
