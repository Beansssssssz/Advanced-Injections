#include <iostream>
#include <Windows.h>
#include <winternl.h>

/* define MessageBoxA prototype */
using PrototypeMessageBox = WINUSERAPI int (WINAPI*)(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);

/* remeber the original address of messageBoxA */
PrototypeMessageBox originalMsgBox = MessageBoxA; 

/* hooked function with "malicious" code that eventually calls the original MessageBoxA */
int WINAPI hookedMessageBox(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
  MessageBoxW(NULL, L"Ola Hooked from a Rogue Senor .o.", L"Ola Senor o/", 0);

  // execute the original NessageBoxA
  return originalMsgBox(hWnd, lpText, lpCaption, uType);
}


int main(int argc, char* argv[]) {
  MessageBoxA(NULL, "Hello Before Hooking", "Hello Before Hooking", 0);

  HINSTANCE imageBase = GetModuleHandleW(NULL);
  if (imageBase == NULL)
  {
    std::cerr << "Failed getting the moudle handle. Error code: "
      << GetLastError() << std::endl;
    return 1;
  }

  PIMAGE_DOS_HEADER dosHeaders = (PIMAGE_DOS_HEADER)imageBase;
  /* the e_lfanew is an offset that combined with the location of the original 
     handle points to where the ntHeaders are located in memory */
  PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((DWORD_PTR)imageBase + dosHeaders->e_lfanew);

  IMAGE_DATA_DIRECTORY importsDirectory = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
  PIMAGE_IMPORT_DESCRIPTOR importDescriptor = NULL;
  importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)(importsDirectory.VirtualAddress + (DWORD_PTR)imageBase);

  LPCSTR libraryName = NULL;
  HMODULE library = NULL;
  PIMAGE_IMPORT_BY_NAME functionName = NULL;
  bool found = false;

  while (importDescriptor->Name != 0 && !found) {
    libraryName = (LPCSTR)importDescriptor->Name + (DWORD_PTR)imageBase;
    library = LoadLibraryA(libraryName);

    if (library == NULL) {
      importDescriptor++;
      continue;
    }

    PIMAGE_THUNK_DATA originalFirstThunk = NULL, firstThunk = NULL;
    originalFirstThunk = (PIMAGE_THUNK_DATA)((DWORD_PTR)imageBase + importDescriptor->OriginalFirstThunk);
    firstThunk = (PIMAGE_THUNK_DATA)((DWORD_PTR)imageBase + importDescriptor->FirstThunk);

    while(originalFirstThunk->u1.AddressOfData != NULL){
      functionName = (PIMAGE_IMPORT_BY_NAME)((DWORD_PTR)imageBase + originalFirstThunk->u1.AddressOfData);

      // find MessageBoxA address
      if (std::string(functionName->Name).compare("MessageBoxA") == 0)
      {
        SIZE_T bytesWritten = 0;
        DWORD oldProtect = 0;
        VirtualProtect((LPVOID)(&firstThunk->u1.Function), 8, PAGE_READWRITE, &oldProtect);

        // swap MessageBoxA address with address of hookedMessageBox
        firstThunk->u1.Function = (DWORD_PTR)hookedMessageBox;
        found = false;
        break;
      }

      originalFirstThunk++;
      firstThunk++;
    }

    importDescriptor++;
  }

  MessageBoxA(NULL, "Hello after Hooking", "Hello after Hooking", 0);


  return 0;

}
