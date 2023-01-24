   global set_gdt
   global reload_segments


; set_gdt(gdt_r) takes a pointer to the GDTR structure
set_gdt:
        lgdt [esp + 4]
        ret


%define system_code_selector (1 << 3)
%define system_data_selector (2 << 3)

; code for reload_segments() taken from
; https://wiki.osdev.org/GDT_Tutorial
reload_segments:
        ; Reload CS register containing code selector
        ; both CS and DS should be zero at this point

        jmp system_code_selector:.reload_cs
    .reload_cs:
        ; Reload data segment registers:
        mov ax, system_data_selector
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        mov ss, ax
        ret