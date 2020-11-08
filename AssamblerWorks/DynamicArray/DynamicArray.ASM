format PE console
entry start
include "win32a.inc"

section ".code" code readable executable
start:

        rdtsc  ; returns the number of clock cycles for the randomizer. Fills in aex with a random value
        mov [Seed],eax ; to put value from eax to Seed

        cinvoke printf, Msg1
        cinvoke scanf, formatStr, Size_A

       ; checking the input array length from the user
        cmp [Size_A],0
        jna notGreaterA
        cmp [Size_A],100
        ja notGreaterA



greater:
        mov eax, [Size_A]
        shl eax, 1 ; multiplication by 2

        invoke VirtualAlloc, NULL, eax, MEM_COMMIT, PAGE_READWRITE ; getting the address of the memory cell that was reserved space from
        mov [Array_A], eax ; put pointer from eax to Array_A
        mov edi, eax  ; put pointer to edi
        cinvoke printf, Msg2, [Size_A]   ; output information about size Array_A
        mov ecx, [Size_A]   ; put Array size to ecx


;     The software loop creates and places values in an array
A:
        push ecx  ; push Array size to Stack
        stdcall Random, 50     ; get random value
        stosw  ; Save value of eax to edi, and , after executing the command, the case is incremented by 2
        cinvoke printf, ItemFormat, eax   ; output this number of Array
        pop ecx  ; to put size of array to ecx
        loop A    ; ecx = ecx-1 if 0 close loop, if not 0, go to A

; input Х
        cinvoke printf, Msg3
        cinvoke scanf, formatStr, input_X

        cmp [input_X],0
        jna notGreaterForX
        cmp [input_X],50
        ja notGreaterForX

        mov ecx, [Size_A]  ; put size Array_A to  ecx
        mov esi, [Array_A]  ; put pointer of Array_A to esi
        xor edx, edx ; clearing the edx

; counting numbers, divisible divisible by x
B:
    lodsw  ;  copies the value from the cell at the esi address to the eax register. after executing the command, the esi is increased by 2

        mov ebx, [input_X]
        xor edx, edx
        div ebx
        cmp edx,0
        jne ok
        inc [Size_B]
ok:
loop B

        ;checking the length of array_B if zero, close the programm
         mov eax, [Size_B]
        cmp eax,0
        je notGreaterForB


        cinvoke printf, Msg4, [Size_B]   ; output information about size Array_B


;         memory allocation for  Array_B
        mov eax, [Size_B]
        shl eax, 1
        invoke VirtualAlloc, NULL, eax, MEM_COMMIT, PAGE_READWRITE
        mov [Array_B], eax
        mov edi, eax





        mov ecx, [Size_A]
        mov esi, [Array_A]
        xor edx, edx

; Re pass through the array_A but now copy the appropriate elements to the array_B
C:
        lodsw
        push eax
        mov ebx, [input_X]
        xor edx, edx
        div ebx
        cmp edx,0
        jne kk
        pop eax
        stosw
kk:
loop C


; Print Array_B
   mov ecx, [Size_B]
   mov esi, [Array_B]
D:
    push ecx
    lodsw
    cinvoke printf, ItemFormat, eax
    pop ecx
loop D
jmp finish


notGreaterForX:
cinvoke printf, Msg6
jmp finish

notGreaterA:
cinvoke printf, Msg5
jmp finish

notGreaterForB:
cinvoke printf, Msg7,[input_X]
jmp finish


finish:
; Clearing Memory
    cinvoke getch
    invoke VirtualFree, [Array_A], NULL, MEM_RELEASE
    invoke VirtualFree, [Array_B], NULL, MEM_RELEASE
    cinvoke exit, NULL


; Рандомайзер
proc Random stdcall uses edx, Range :dword
    imul eax, [Seed], 08088405h
    inc eax
    mov [Seed], eax
    mul dword [Range]
    mov eax, edx
    ret
endp

section ".data" data readable

        Msg1 db "Input length Array (1-100): ",0

        Msg2 db "Create Array_A [%u]:", 13, 10, 0

        Msg3 db 13,10, "Please, input number: ",0

        Msg4 db 13,10, "Create Array_B [%u]:",13,10,0

        Msg5 db 13,10,"Incorrect length Array",0
        Msg6 db 13,10,"Incorrect X",0
        Msg7 db 13,10, "there are no numbers in array_A that are completely divisible by %u",0


        ItemFormat db "%4d", 0

        formatStr db "%d",0



section ".bss" data readable writeable

        Seed rd 1
        Size_A rd 1
        Array_A rd 1

        input_X rd 1
        Size_B rd 1
        Array_B rd 1


section ".idata" import data readable

        library kernel32,"kernel32.dll",\
                msvcrt,"msvcrt.dll"

        include "api/kernel32.inc"

        import msvcrt,\
                printf, "printf",\
                getch, "_getch",\
                scanf,"scanf",\
                exit, "_exit"