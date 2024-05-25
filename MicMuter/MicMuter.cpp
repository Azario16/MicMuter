#include <windows.h>
#include <shellapi.h>
#include "resource.h"
#include <string>
#include <mmdeviceapi.h>
#include <endpointvolume.h>


const int WM_HOTKEY_MSG_ID = WM_USER + 1;
const int WM_TRAYICON_MSG_ID = WM_USER + 2;

HINSTANCE hInstance;
HWND hWnd;
HWAVEIN hWaveIn;

// Create the tray icon
NOTIFYICONDATA nid;
static HICON hIcon1;
static HICON hIcon2;
static bool isIcon1 = true;

// Const microphone
IMMDeviceEnumerator* enumerator = nullptr;
IMMDevice* defaultDevice = nullptr;
IAudioEndpointVolume* endpointVolume = nullptr;
BOOL muteState;


void initializeMicComLib() {
    // Initialize COM library
    CoInitialize(NULL);

    // Create COM objects
    CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID*)&enumerator);
    enumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &defaultDevice);
    defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (LPVOID*)&endpointVolume);
}

void showMuteIcon() {
    static bool isIcon1 = true;
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON;
    nid.hIcon = hIcon2;
    Shell_NotifyIcon(NIM_MODIFY, &nid);

    // Display a debug message
    OutputDebugString("Show unmute microphone\n");
}

void showUnmuteIcon() {
    static bool isIcon1 = true;
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON;
    nid.hIcon = hIcon1;
    Shell_NotifyIcon(NIM_MODIFY, &nid);

    // Display a debug message
    OutputDebugString("Show mute microphone\n");
}


void getMuteState() {
    endpointVolume->GetMute(&muteState);
}

void toggleMicrophoneMute()
{
    endpointVolume->GetMute(&muteState);
    endpointVolume->SetMute(!muteState, nullptr);

    OutputDebugString(("key pressed " + std::to_string(muteState) + "\n ").c_str());

    if (muteState) {
        showUnmuteIcon();
        endpointVolume->SetMasterVolumeLevelScalar(0.98, NULL);
    }
    else {
        showMuteIcon();
        endpointVolume->SetMasterVolumeLevelScalar(0, NULL);
    }
}


LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //OutputDebugString(("key pressed " + std::to_string(wParam) + "\n ").c_str());

    switch (uMsg)
    {
    case WM_HOTKEY:
        if (wParam == 1)
        {
            toggleMicrophoneMute();
        }
        break;

    case WM_TRAYICON_MSG_ID:
        switch (lParam)
        {
        case WM_RBUTTONUP:
            // Show a context menu
        {
            POINT pt;
            GetCursorPos(&pt);
            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, 1, "Exit");
            SetForegroundWindow(hWnd);
            int command = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, hWnd, nullptr);
            if (command == 1)
            {
                OutputDebugString("command = 1 \n");
                PostQuitMessage(0);
            }
            DestroyMenu(hMenu);
        }
        case WM_LBUTTONUP:
        case NIN_SELECT:
            Shell_NotifyIcon(NIM_SETFOCUS, &nid);
            break;
            break;
        }
        break;

    case WM_DESTROY:
        OutputDebugString("WM_DESTROY\n");
        PostQuitMessage(0);
        break;
    case WM_CLOSE:
        OutputDebugString("WM_CLOSE\n");
        DestroyWindow(hWnd);
        break;

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Store the instance handle
    ::hInstance = hInstance;

    initializeMicComLib();

    // Register the main window class
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "IconChangeWindowClass";
    RegisterClass(&wc);

    // Create a hidden window
    hWnd = CreateWindowEx(0, wc.lpszClassName, NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);


    // Load the icons
    hIcon1 = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    hIcon2 = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON2));

    getMuteState();

    // Create the tray icon
    nid.cbSize = sizeof(nid);
    nid.hWnd = hWnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON_MSG_ID;
    nid.hIcon = !muteState ? hIcon1 : hIcon2;
    Shell_NotifyIcon(NIM_ADD, &nid);

    // Register the hotkey
    BOOL registered = RegisterHotKey(hWnd, 1, MOD_SHIFT, VK_TAB);
    if (!registered)
    {
        MessageBox(hWnd, "Failed to register hotkey!", "Error", MB_OK | MB_ICONERROR);
    }

    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Remove the tray icon
    OutputDebugString("Remove \n");
    Shell_NotifyIcon(NIM_DELETE, &nid);

    CoUninitialize();
    return 0;
}
