#include <iostream>

int inlineAsm() {
  int a = 10, b = 20, result;

  // Inline assembly to add 'a' and 'b'
  __asm {
    mov eax, dword ptr[a]    // Move the value of 'a' into register eax
    add eax, b   // Add the value of 'b' to register eax
    mov result, eax // Move the result to the variable 'result'
  };

  std::cout << "Result: " << result << std::endl;

  return 0;
}
