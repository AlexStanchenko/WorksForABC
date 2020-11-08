format PE console
entry start
include "win32a.inc"

section ".code" code readable executable
start:

;print message about program
cinvoke printf, msgCreate

;pring first three number of sequence
cinvoke printf,msgNumber32,0,dword[pPrevPrev]
cinvoke printf,msgNumber32,1,dword[pPrev]
cinvoke printf,msgNumber32,2,dword[pCurr]


;.........infinity loop for search numbers of sequence <= 2^32...........................
C:

        ; P(n) = P(n-3) + P(n-2)
        mov eax,dword [pPrevPrev]
        add eax,dword [pPrev]
        mov dword [pNext],eax
        jc secondPart; this predicate is true, if CF = 1, =>  P(n) > 2^32  => break loop

        ; P(n-3) = P(n-2)
        mov eax, dword[pPrev]
        mov dword[pPrevPrev],eax

        ; P(n-2) = P(n-1)
        mov eax,dword[pCurr]
        mov dword[pPrev],eax

        ;P(n-1) = P(n)
        mov eax,dword[pNext]
        mov dword [pCurr],eax


        cinvoke printf,msgNumber32,[count],dword[pCurr];print number of sequence

        ; we add 1 to count and continue loop
        add [count],1  ; add 1 to count
        mov ecx,2  ; iterator for infinity loop

loop C ; ecx=ecx-1. If ecx >0, continue loop


secondPart:
        ;print message about count of numbers of secuence <=2^32
        cinvoke printf,msgCount32,[count]

;........Continue infitiny loop for search numbers of secuence <=2^64....................
D:
        ; jump to code for search next number
        jmp for64
; renurn from search next number
end64:

        ; we add 1 to count and continue loop
        add [count],1  ; add 1 to count
        mov ecx,2  ; iterator for infinity loop

loop D ; ecx=ecx-1. If ecx >0, continue loop


for64:

        ; clear high 32bits of pNext number
        mov dword[pNext+4],0

        ; P(n) = P(n-3) + P(n-2)
        mov eax,dword [pPrevPrev]
        add eax,dword [pPrev]
        mov dword [pNext],eax
        ; Add one high bit, if CF=1
        adc dword [pNext+4], edx

        ; Clear CF
        CLC

        ; Addition of the high bits
        mov eax,dword [pNext+4]
        add eax, dword [pPrevPrev+4]
        add eax, dword [pPrev+4]
        mov dword [pNext+4],eax

        ; if CF=1 => number >2^64 => break loop and finish program
        jc finish



        ; P(n-3) = P(n-2)
        mov eax,dword[pPrev]
        mov dword [pPrevPrev],eax
        mov eax, dword[pPrev+4]
        mov dword [pPrevPrev+4],eax

        ; P(n-2) = P(n-1)
        mov eax,dword[pCurr]
        mov dword [pPrev],eax
        mov eax, dword[pCurr+4]
        mov dword [pPrev+4],eax

        ; P(n-1) = P(n)
        mov eax,dword[pNext]
        mov dword[pCurr],eax
        mov eax, dword[pNext+4]
        mov dword[pCurr+4],eax

        ;print message about number of sequence
        cinvoke printf, msgNumber64,[count],dword[pNext+4],dword[pNext]

        ; end search number
        jmp end64

finish:

        ;print message about count of numbers of sequence <=2^64
        cinvoke printf,msgCount64,[count]

        cinvoke getch
        cinvoke exit, NULL


section ".data" data readable

        msgNumber32 db 13,10, "[%u] Low 32bits: %10u",0
        msgNumber64 db 13,10, "[%u]  High 32bits: %10u  Low 32bits: %10u",0
        msgCreate db 13,10, "Padawan sequence from 1 to machine word",13,10,"Usually the machine word is processor bit rate ",0
        msgCount32 db 13,10,"If your system is 32 bits, then the Padawan sequence from 1 to the machine word contains %u numbers",13,10,13,10,0
        msgCount64 db 13,10,"If your system is 64 bits, then the Padawan sequence from 1 to the machine word contains %u numbers",0


section ".bss" data readable writeable

        pPrevPrev dq 1
        pPrev dq 1
        pCurr dq 1
        pNext dq 1
        count dd 3


section ".idata" import data readable

        library kernel32,"kernel32.dll",\
                msvcrt,"msvcrt.dll"

        include "api/kernel32.inc"

        import msvcrt,\
                printf, "printf",\
                getch, "_getch",\
                scanf,"scanf",\
                exit, "_exit"