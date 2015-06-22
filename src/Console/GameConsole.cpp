#include "GameConsole.h"
#include "../Utils/VersionInfo.h"
#include "../CommandMap.h"
#include <sstream>

void GameConsole::startIRCBackend()
{
	GameConsole::Instance().ircBackend = std::make_unique<IRCBackend>();
}

GameConsole::GameConsole()
{
	for (int i = 0; i < numOfLinesBuffer; i++) {
		queue.push_back("");
	}

	initPlayerName();
	DirectXHook::hookDirectX();
	KeyboardHook::setHook();
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&startIRCBackend, 0, 0, 0);

	pushLineFromGameToUI("ElDewrito Version: " + Utils::Version::GetVersionString() + " Build Date: " + __DATE__ + " " + __TIME__);
	pushLineFromGameToUI("Enter /help or /help <command> to get started!");
}

bool GameConsole::isConsoleShown() {
	return boolShowConsole;
}

int GameConsole::getMsSinceLastReturnPressed()
{
	return GetTickCount() - lastTimeConsoleShown;
}

int GameConsole::getMsSinceLastConsoleOpen()
{
	return GetTickCount() - lastTimeConsoleShown;
}

void GameConsole::peekConsole()
{
	lastTimeConsoleShown = GetTickCount();
}

void GameConsole::hideConsole()
{
	lastTimeConsoleShown = GetTickCount();
	boolShowConsole = false;
	inputLine.clear();
	
	// Enables game keyboard input and disables our keyboard hook

	RAWINPUTDEVICE Rid;
	Rid.usUsagePage = 0x01;
	Rid.usUsage = 0x06;
	Rid.dwFlags = RIDEV_REMOVE;
	Rid.hwndTarget = 0;

	if (!RegisterRawInputDevices(&Rid, 1, sizeof(Rid))) {
		pushLineFromGameToUI("Unregistering keyboard failed");
	}
}

void GameConsole::showConsole()
{
	boolShowConsole = true;
	capsLockToggled = GetKeyState(VK_CAPITAL) & 1;

	// Disables game keyboard input and enables our keyboard hook
	RAWINPUTDEVICE Rid;
	Rid.usUsagePage = 0x01;
	Rid.usUsage = 0x06;
	Rid.dwFlags = RIDEV_NOLEGACY; // adds HID keyboard and also ignores legacy keyboard messages
	Rid.hwndTarget = 0;

	if (!RegisterRawInputDevices(&Rid, 1, sizeof(Rid))) {
		pushLineFromGameToUI("Registering keyboard failed");
	}
}

void GameConsole::virtualKeyCallBack(USHORT vKey)
{
	if (!isConsoleShown())
	{
		if (vKey == VK_RETURN)
		{
			showConsole();
		}
		return;
	}

	switch (vKey)
	{
	case VK_RETURN:
		if (!inputLine.empty())
		{
			pushLineFromKeyboardToGame(inputLine);
		}
		hideConsole();
		lastTimeReturnPressed = GetTickCount();
		break;

	case VK_ESCAPE:
		hideConsole();
		break;

	case VK_BACK:
		if (!inputLine.empty())
		{
			inputLine.pop_back();
		}
		break;

	case VK_CAPITAL:
		capsLockToggled = !capsLockToggled;
		break;

	case VK_PRIOR:
		if (queueStartIndexForUI < numOfLinesBuffer - numOfLinesToShow)
		{
			queueStartIndexForUI++;
		}
		break;

	case VK_NEXT:
		if (queueStartIndexForUI > 0)
		{
			queueStartIndexForUI--;
		}
		break;

	default:
		WORD buf;
		BYTE keysDown[256] = {};

		if (GetAsyncKeyState(VK_SHIFT) & 0x8000) // 0x8000 = 0b1000000000000000
		{
			keysDown[VK_SHIFT] = 0x80; // sets highest-order bit to 1: 0b10000000
		}

		if (capsLockToggled)
		{
			keysDown[VK_CAPITAL] = 0x1; // sets lowest-order bit to 1: 0b00000001
		}

		int retVal = ToAscii(vKey, 0, keysDown, &buf, 0);

		if (retVal == 1)
		{
			inputLine += buf & 0x00ff;
		}
		else if (retVal == 2)
		{
			inputLine += buf >> 8;
			inputLine += buf & 0x00ff;
		}
		break;
	}
}

void GameConsole::pushLineFromKeyboardToGame(std::string line)
{
	if (line.find("/") == 0)
	{
		pushLineFromGameToUI(line);
		line.erase(0, 1);
		pushLineFromGameToUIMultipleLines(Modules::CommandMap::Instance().ExecuteCommand(line));
	}
	else
	{
		std::string preparedLineForIRC = playerName;
		preparedLineForIRC += ": ";
		preparedLineForIRC += line;
		sendThisLineToIRCServer = preparedLineForIRC;
		pushLineFromGameToUI(preparedLineForIRC);
	}
}

void GameConsole::initPlayerName()
{
	wchar_t* inGameName = (wchar_t*)0x19A03E8; // unicode
	std::wstring toWstr(inGameName);
	std::string toStr(toWstr.begin(), toWstr.end());
	playerName = toStr;
}

void GameConsole::pushLineFromGameToUI(std::string line)
{
	for (int i = numOfLinesBuffer - 1; i > 0; i--)
	{
		queue.at(i) = queue.at(i - 1);
	}
	queue.at(0) = line;

	peekConsole();
}

void GameConsole::pushLineFromGameToUIMultipleLines(std::string multipleLines)
{
	std::vector<std::string> linesVector;
	split(multipleLines, '\n', linesVector);

	for (std::string line : linesVector)
	{
		pushLineFromGameToUI(line);
	}
}

std::string GameConsole::at(int i)
{
	return queue.at(i);
}

std::vector<std::string>& GameConsole::split(const std::string &s, char delim, std::vector<std::string> &elems)
{
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

void GameConsole::checkForReturnKey()
{
	if ((GetAsyncKeyState(VK_RETURN) & 0x8000) && getMsSinceLastReturnPressed() > 100) {
		showConsole();
		lastTimeReturnPressed = GetTickCount();
	}
}