/* Copyright Philipp Roser -- see LICENSE for license */

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <time.h>
#include <string>
#include "resource.h"



#define ID_SECONDS_FIELD 101 
#define ID_MINUTES_FIELD 102 // Not Implemented

#define ID_START_BUTTON 201
#define ID_STOP_BUTTON 202

#define ID_TIMER 301


HBRUSH _bgBrushTimerRunning_;
HBRUSH _bgBrushTimerStopped_;

bool _isRunning_ = false;
int _durationInSec_ = 0;




LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void SendCtrlSInput();
bool DurationInputIsValid(HWND);
void SetDurationFromInput(HWND);
std::wstring SystemTimeToString(SYSTEMTIME*);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	const wchar_t CLASS_NAME[] = L"Unsavoury Auto Save";

	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(ID_GEAR_ICON));

	RegisterClass(&wc); 

	HWND hwnd = CreateWindowEx(
		0,
		CLASS_NAME,
		L"Unsavoury Auto Save",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 400, 200,
		NULL, NULL, hInstance, NULL
	);

	if (hwnd == NULL)
		return 0;

	ShowWindow(hwnd, nCmdShow);

	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}



LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:

		_isRunning_ = false;

		_bgBrushTimerRunning_ = CreateSolidBrush(RGB(128, 128, 128));
		_bgBrushTimerStopped_ = CreateSolidBrush(RGB(224, 224, 224));

		CreateWindowEx(
			WS_EX_CLIENTEDGE,
			L"EDIT",
			L"",
			WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL,
			50, 50, 100, 25,
			hwnd,
			(HMENU)ID_SECONDS_FIELD, // control ID
			(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
			NULL
		);

		CreateWindow(
			L"Button",
			L"Start",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			250, 25, 100, 30,
			hwnd,
			(HMENU)ID_START_BUTTON,
			(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
			NULL
		);

		CreateWindow(
			L"Button",
			L"Stop",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			250, 75, 100, 30,
			hwnd,
			(HMENU)ID_STOP_BUTTON,
			(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
			NULL
		);

		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_START_BUTTON:
			KillTimer(hwnd, ID_TIMER);
			SetDurationFromInput(hwnd);
			SetTimer(hwnd, ID_TIMER, _durationInSec_ * 1000, NULL);
			_isRunning_ = true;
			break;

		case ID_STOP_BUTTON:
			KillTimer(hwnd, ID_TIMER);
			_isRunning_ = false;
			break;
		}

		InvalidateRect(hwnd, NULL, TRUE);

		return 0;

	case WM_TIMER: 
		SendCtrlSInput();
		SetTimer(hwnd, ID_TIMER, _durationInSec_ * 1000, NULL);
		InvalidateRect(hwnd, NULL, TRUE);
		return 0;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		if (_isRunning_)
			FillRect(hdc, &ps.rcPaint, _bgBrushTimerRunning_);
		else
			FillRect(hdc, &ps.rcPaint, _bgBrushTimerStopped_);

		SetTextColor(hdc, RGB(0, 0, 0));
		SetBkMode(hdc, TRANSPARENT);

		const wchar_t* text = L"Seconds between Ctrl-S:";
		TextOut(hdc, 50, 25, text, wcslen(text));

		if (_isRunning_)
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			std::wstring timeString = SystemTimeToString(&st);
			std::wstring nextSaveText = L"Last Ctrl-S at: ";
			std::wstring combinedString = (nextSaveText + timeString);
			const wchar_t* combinedAsCString = combinedString.c_str();
			TextOut(hdc, 50, 100, combinedAsCString, wcslen(combinedAsCString));
		}

		EndPaint(hwnd, &ps);
		return 0;
	}

	case WM_DESTROY:
		KillTimer(hwnd, ID_TIMER);
		PostQuitMessage(0);
		return 0;
		return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);

}

void SendCtrlSInput()
{
	INPUT input[4] = {};
	
	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = VK_CONTROL;
	
	input[1].ki.wVk = 'S';
	input[1].type = INPUT_KEYBOARD;

	input[2].type = INPUT_KEYBOARD;
	input[2].ki.wVk = 'S';
	input[2].ki.dwFlags = KEYEVENTF_KEYUP;
	
	input[3].type = INPUT_KEYBOARD;
	input[3].ki.wVk = VK_CONTROL;
	input[3].ki.dwFlags = KEYEVENTF_KEYUP;
	
	SendInput(4, input, sizeof(INPUT));

	OutputDebugString(L"Input sent.\n");

	wchar_t debugString[100];
	swprintf(debugString, 64, L"Next timer in %d seconds.", _durationInSec_);
	OutputDebugString(debugString);

	return;
}

bool DurationInputIsValid(HWND hwnd)
{
	wchar_t buffer[32];
	GetDlgItemText(hwnd, ID_SECONDS_FIELD, buffer, sizeof(buffer) / sizeof(wchar_t));
	
	for (int i = 0; i < sizeof(buffer); i++)
	{
		if (buffer[i] == '\0')
			break;
		if (!std::isdigit(buffer[i]))
		{
			OutputDebugString(L"BAD INPUT.");
			return false;
		}
	}

	return true;
}

void SetDurationFromInput(HWND hwnd)
{
	// currently only minutes

	wchar_t buffer[32];
	GetDlgItemText(hwnd, ID_SECONDS_FIELD, buffer, sizeof(buffer) / sizeof(wchar_t));

	_durationInSec_ = _wtoi(buffer);
	if (_durationInSec_ < 1)
		_durationInSec_ = 1;
	return;
}

std::wstring SystemTimeToString(SYSTEMTIME* st)
{
	wchar_t timeString[16];
	swprintf(timeString, sizeof(timeString) / sizeof(wchar_t),
		L"%02d:%02d:%02d\n", st->wHour, st->wMinute, st->wSecond);
	return timeString;
}
