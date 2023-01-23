
   extern kernel_main
   extern kernel_stack_top
   global kstart

[SECTION .text]
kstart:
        mov eax, kernel_stack_top
        mov esp, eax

        call kernel_main

   .halted_loop:
        hlt
        jmp short .halted_loop