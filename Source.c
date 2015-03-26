#include <Windows.h>
#include <stdio.h>
#include <time.h>
#include <string.h>


char ToLower(char);
char ToUpper(char);
int SendInputString(char*);
int CopySelectedText(DWORD);

LPCWSTR clipBoard[10];

struct tm lastPress;



//The function that implements the key logging functionality
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	// Declare a pointer to the KBDLLHOOKSTRUCT
	KBDLLHOOKSTRUCT *pKeyBoard = (KBDLLHOOKSTRUCT *)lParam;
	switch (wParam)
	{
	case WM_KEYDOWN: // When the key has been pressed and released
	{
						 if (pKeyBoard->vkCode >= 0x30 && pKeyBoard->vkCode <= 0x39) {
							 if (GetAsyncKeyState(VK_CONTROL)){
								 if (GetAsyncKeyState(VK_LSHIFT)) { //Copy the value

									 CopySelectedText(pKeyBoard->vkCode);

								 }
								 else {
									 //Paste The Value
									 int index = pKeyBoard->vkCode - 0x30;
									 SendInputString(clipBoard[index]);
								 }
								 return 1;
							 }

						 }
						 
	}
		break;
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	default:
		return CallNextHookEx(NULL, nCode, wParam, lParam);
		break;
	}

	//do something with the pressed key here


	//according to winapi all functions which implement a hook must return by calling next hook
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}
// Step 4: the Window Procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rect;
	switch (msg)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		free(clipBoard);
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		GetClientRect(hwnd, &rect);
		EndPaint(hwnd, &ps);
		return 0;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{

	//Initializing clipBoard array
	for (int i = 0; i < 10; i++) {
		*(clipBoard + i) = (LPCWSTR)calloc(256, sizeof(char));
	}

	WNDCLASSEX wc;


	//Initializing lastPress
	time_t rawtime;
	time(&rawtime);
	localtime_s(&lastPress, &rawtime);

	//Step 1: Registering the Window Class
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"myApp";
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Window Registration Failed!", L"Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}


	//Retrieve the applications instance
	HINSTANCE instance = GetModuleHandle(0);
	//Set a global Windows Hook to capture keystrokes using the function declared above
	HHOOK test1 = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, instance, 0);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	UnhookWindowsHookEx(test1);
	return 0;
}

int SendInputString(LPCWSTR string) {

	//Gets a string and sends every char of it as Input
	int index = 0;

	while (string[index]){
		INPUT input;
		input.type = INPUT_KEYBOARD;
		input.ki.dwExtraInfo = 0;
		input.ki.wVk = 0;
		input.ki.wScan = string[index++];
		input.ki.time = 0;
		input.ki.dwFlags = KEYEVENTF_UNICODE;
		SendInput(1, &input, sizeof(INPUT));
		input.ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(1, &input, sizeof(INPUT));
	}

	return 0;
}

char ToLower(char ch) {
	if (ch >= 65 && ch <= 90)
		return ch + 32;
	return ch;
}

char ToUpper(char ch) {
	if (ch >= 97 && ch <= 122) {
		return ch - 32;
	}
	return ch;
}

int CopySelectedText(DWORD vkCode) {
	//copies the selected text to clipboard by sending CTRL + C to the window

	if (OpenClipboard(NULL)) {

		struct tm curPress;

		time_t rawtime = time(&rawtime);

		localtime_s(&curPress, &rawtime);

		//Checks if a second has passes since last press, to avoid multiple/long presses
		if (abs(curPress.tm_sec - lastPress.tm_sec) >= 1) {

			lastPress = curPress;

			INPUT cntrl, cPress;

			memset(&cntrl, 0, sizeof(INPUT));
			memset(&cPress, 0, sizeof(INPUT));

			cntrl.type = INPUT_KEYBOARD;
			cPress.type = INPUT_KEYBOARD;

			cntrl.ki.wVk = VK_CONTROL;
			cPress.ki.wVk = 0x43; // 0x43 = C key

			cntrl.ki.dwFlags = KEYEVENTF_UNICODE;
			cntrl.ki.dwFlags = KEYEVENTF_UNICODE;

			SendInput(1, &cntrl, sizeof(INPUT));
			SendInput(1, &cPress, sizeof(INPUT));

			cntrl.ki.dwFlags = KEYEVENTF_KEYUP;
			cPress.ki.dwFlags = KEYEVENTF_KEYUP;

			SendInput(1, &cPress, sizeof(INPUT));
			SendInput(1, &cntrl, sizeof(INPUT));
			CloseClipboard();
			Sleep(100); //Wait, or else the clipboard won't be updated by the time it is used
		}
		if (OpenClipboard(NULL)){
			HANDLE hClipboardData = GetClipboardData(CF_UNICODETEXT);

			
			LPCWSTR clipBoardText;
			clipBoardText = (LPCWSTR)hClipboardData;

			int index = vkCode - 0x30; //Index in array

			if (clipBoard[index]) {
				memset(clipBoard[index], 0, 256);
			}

			wcscpy_s(clipBoard[index], 256, clipBoardText);

			CloseClipboard();

			return 0;

		}
	}
	return 1;
}