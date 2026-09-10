section .text
  global _start

_start:
  mov rdi, [rsp]
  lea rsi, [rsp + 8]
  lea rdx, [rsi + rdi*8 + 8]

  extern main
  call main

  mov rdi, rax
  mov rax, 60
  syscall
