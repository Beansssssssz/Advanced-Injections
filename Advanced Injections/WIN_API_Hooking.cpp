#include <iostream>
#include <Windows.h>

FARPROC messageBoxAddress = NULL;
SIZE_T bytesWritten = 0;
char messageBoxOriginalBytes[6] = {};

int __stdcall HookedMessageBox(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {

  // print intercepted values from the MessageBoxA function
  std::cout << "Oh ai from the hooked function\n";
  std::cout << "Text: " << (LPCSTR)lpText << "\nCaption: " << (LPCSTR)lpCaption << std::endl;

  // unpatch MessageBoxA
  WriteProcessMemory(GetCurrentProcess(), (LPVOID)messageBoxAddress,
    messageBoxOriginalBytes, sizeof(messageBoxOriginalBytes), &bytesWritten);

  return MessageBoxA(NULL, lpText, lpCaption, uType);
  //return IDOK;
}

  /// <summary>
/// this function hooks a mesageBoxA function into my personal function.
/// </summary>
/// <returns>0  if no errors occured, otherwise returns 1</returns>
int main() {
  /* show messageBox before hooking */
  MessageBoxA(NULL, "hi", "hi", MB_OK);

  HINSTANCE library = LoadLibraryA("user32.dll");
  SIZE_T bytesRead = 0;

  if (library == NULL) {
    std::cerr << "couldnt load the library. Error code: " <<
      GetLastError() << std::endl;
    return 1;
  }

  /* get address of the MessageBox function in memory */
  messageBoxAddress = GetProcAddress(library, "MessageBoxA");
  if (messageBoxAddress == NULL) {
    std::cerr << "couldnt get the function address. Error code: " <<
      GetLastError() << std::endl;
    return 1;
  }

  /* copies the memory bytes from this process starting at inputed address, for size of messageBoxOriginalBytes */
  if (ReadProcessMemory(GetCurrentProcess(), messageBoxAddress,
    messageBoxOriginalBytes, sizeof(messageBoxOriginalBytes), &bytesRead) == FALSE)
  {
    std::cerr << "couldnt read the current procces memory. Error code: " <<
      GetLastError() << std::endl;
    return 1;
  }

  /* create a patch "push <address of new MessageBoxA); ret" */
  void* hookedMessageBoxAddress = &HookedMessageBox;
  char patch[6] = { 0 };
  memcpy_s(patch, 1, "\x68", 1);
  memcpy_s(patch + 1, 4, &hookedMessageBoxAddress, 4);
  memcpy_s(patch + 5, 1, "\xC3", 1);

  /* write the current new the MessageBoxA */
  WriteProcessMemory(GetCurrentProcess(), (LPVOID)messageBoxAddress, patch, sizeof(patch), &bytesWritten);

  /* run the messageBox after the hooking is applied*/
  MessageBoxA(NULL, "hi", "hi", MB_OK);

  return 0;
}
