; 1 "dct64_mmx.S"
; 1 "<built-in>"
; 1 "<command line>"
; 1 "dct64_mmx.S"
; 9 "dct64_mmx.S"
; 1 "mangle.h" 1
; 13 "mangle.h"
; 1 "config.h" 1
; 14 "mangle.h" 2
; 1 "intsym.h" 1
; 15 "mangle.h" 2
; 10 "dct64_mmx.S" 2

%include "asm_nasm.inc"

_sym_mangle costab_mmxsse
_sym_mangle dct64_mmx
_sym_mangle dct64_MMX

EXTERN costab_mmxsse

SECTION .text

 ALIGN 32
GLOBAL dct64_mmx
dct64_mmx:

 xor  ecx,ecx
GLOBAL dct64_MMX
dct64_MMX:
 push  ebx
 push  esi
 push  edi
 sub  esp,256



 mov  eax, [esp+280]
 fld  dword [eax]
 lea  edx, [esp+128]
 fadd  dword [eax+124]
 mov  esi, [esp+272]
 fstp  dword [edx]
 mov  edi, [esp+276]
 fld  dword [eax+4]



 lea  ebx, [costab_mmxsse]

 fadd  dword [eax+120]
 or  ecx,ecx
 fstp  dword [edx+4]
 fld  dword [eax]
 mov  ecx,esp
 fsub  dword [eax+124]
 fmul  dword [ebx]
 fstp  dword [edx+124]
 fld  dword [eax+4]
 fsub  dword [eax+120]
 fmul  dword [ebx+4]
 fstp  dword [edx+120]
 fld  dword [eax+8]
 fadd  dword [eax+116]
 fstp  dword [edx+8]
 fld  dword [eax+12]
 fadd  dword [eax+112]
 fstp  dword [edx+12]
 fld  dword [eax+8]
 fsub  dword [eax+116]
 fmul  dword [ebx+8]
 fstp  dword [edx+116]
 fld  dword [eax+12]
 fsub  dword [eax+112]
 fmul  dword [ebx+12]
 fstp  dword [edx+112]
 fld  dword [eax+16]
 fadd  dword [eax+108]
 fstp  dword [edx+16]
 fld  dword [eax+20]
 fadd  dword [eax+104]
 fstp  dword [edx+20]
 fld  dword [eax+16]
 fsub  dword [eax+108]
 fmul  dword [ebx+16]
 fstp  dword [edx+108]
 fld  dword [eax+20]
 fsub  dword [eax+104]
 fmul  dword [ebx+20]
 fstp  dword [edx+104]
 fld  dword [eax+24]
 fadd  dword [eax+100]
 fstp  dword [edx+24]
 fld  dword [eax+28]
 fadd  dword [eax+96]
 fstp  dword [edx+28]
 fld  dword [eax+24]
 fsub  dword [eax+100]
 fmul  dword [ebx+24]
 fstp  dword [edx+100]
 fld  dword [eax+28]
 fsub  dword [eax+96]
 fmul  dword [ebx+28]
 fstp  dword [edx+96]
 fld  dword [eax+32]
 fadd  dword [eax+92]
 fstp  dword [edx+32]
 fld  dword [eax+36]
 fadd  dword [eax+88]
 fstp  dword [edx+36]
 fld  dword [eax+32]
 fsub  dword [eax+92]
 fmul  dword [ebx+32]
 fstp  dword [edx+92]
 fld  dword [eax+36]
 fsub  dword [eax+88]
 fmul  dword [ebx+36]
 fstp  dword [edx+88]
 fld  dword [eax+40]
 fadd  dword [eax+84]
 fstp  dword [edx+40]
 fld  dword [eax+44]
 fadd  dword [eax+80]
 fstp  dword [edx+44]
 fld  dword [eax+40]
 fsub  dword [eax+84]
 fmul  dword [ebx+40]
 fstp  dword [edx+84]
 fld  dword [eax+44]
 fsub  dword [eax+80]
 fmul  dword [ebx+44]
 fstp  dword [edx+80]
 fld  dword [eax+48]
 fadd  dword [eax+76]
 fstp  dword [edx+48]
 fld  dword [eax+52]
 fadd  dword [eax+72]
 fstp  dword [edx+52]
 fld  dword [eax+48]
 fsub  dword [eax+76]
 fmul  dword [ebx+48]
 fstp  dword [edx+76]
 fld  dword [eax+52]
 fsub  dword [eax+72]
 fmul  dword [ebx+52]
 fstp  dword [edx+72]
 fld  dword [eax+56]
 fadd  dword [eax+68]
 fstp  dword [edx+56]
 fld  dword [eax+60]
 fadd  dword [eax+64]
 fstp  dword [edx+60]
 fld  dword [eax+56]
 fsub  dword [eax+68]
 fmul  dword [ebx+56]
 fstp  dword [edx+68]
 fld  dword [eax+60]
 fsub  dword [eax+64]
 fmul  dword [ebx+60]
 fstp  dword [edx+64]

 fld  dword [edx]
 fadd  dword [edx+60]
 fstp  dword [ecx]
 fld  dword [edx+4]
 fadd  dword [edx+56]
 fstp  dword [ecx+4]
 fld  dword [edx]
 fsub  dword [edx+60]
 fmul  dword [ebx+64]
 fstp  dword [ecx+60]
 fld  dword [edx+4]
 fsub  dword [edx+56]
 fmul  dword [ebx+68]
 fstp  dword [ecx+56]
 fld  dword [edx+8]
 fadd  dword [edx+52]
 fstp  dword [ecx+8]
 fld  dword [edx+12]
 fadd  dword [edx+48]
 fstp  dword [ecx+12]
 fld  dword [edx+8]
 fsub  dword [edx+52]
 fmul  dword [ebx+72]
 fstp  dword [ecx+52]
 fld  dword [edx+12]
 fsub  dword [edx+48]
 fmul  dword [ebx+76]
 fstp  dword [ecx+48]
 fld  dword [edx+16]
 fadd  dword [edx+44]
 fstp  dword [ecx+16]
 fld  dword [edx+20]
 fadd  dword [edx+40]
 fstp  dword [ecx+20]
 fld  dword [edx+16]
 fsub  dword [edx+44]
 fmul  dword [ebx+80]
 fstp  dword [ecx+44]
 fld  dword [edx+20]
 fsub  dword [edx+40]
 fmul  dword [ebx+84]
 fstp  dword [ecx+40]
 fld  dword [edx+24]
 fadd  dword [edx+36]
 fstp  dword [ecx+24]
 fld  dword [edx+28]
 fadd  dword [edx+32]
 fstp  dword [ecx+28]
 fld  dword [edx+24]
 fsub  dword [edx+36]
 fmul  dword [ebx+88]
 fstp  dword [ecx+36]
 fld  dword [edx+28]
 fsub  dword [edx+32]
 fmul  dword [ebx+92]
 fstp  dword [ecx+32]

 fld  dword [edx+64]
 fadd  dword [edx+124]
 fstp  dword [ecx+64]
 fld  dword [edx+68]
 fadd  dword [edx+120]
 fstp  dword [ecx+68]
 fld  dword [edx+124]
 fsub  dword [edx+64]
 fmul  dword [ebx+64]
 fstp  dword [ecx+124]
 fld  dword [edx+120]
 fsub  dword [edx+68]
 fmul  dword [ebx+68]
 fstp  dword [ecx+120]
 fld  dword [edx+72]
 fadd  dword [edx+116]
 fstp  dword [ecx+72]
 fld  dword [edx+76]
 fadd  dword [edx+112]
 fstp  dword [ecx+76]
 fld  dword [edx+116]
 fsub  dword [edx+72]
 fmul  dword [ebx+72]
 fstp  dword [ecx+116]
 fld  dword [edx+112]
 fsub  dword [edx+76]
 fmul  dword [ebx+76]
 fstp  dword [ecx+112]
 fld  dword [edx+80]
 fadd  dword [edx+108]
 fstp  dword [ecx+80]
 fld  dword [edx+84]
 fadd  dword [edx+104]
 fstp  dword [ecx+84]
 fld  dword [edx+108]
 fsub  dword [edx+80]
 fmul  dword [ebx+80]
 fstp  dword [ecx+108]
 fld  dword [edx+104]
 fsub  dword [edx+84]
 fmul  dword [ebx+84]
 fstp  dword [ecx+104]
 fld  dword [edx+88]
 fadd  dword [edx+100]
 fstp  dword [ecx+88]
 fld  dword [edx+92]
 fadd  dword [edx+96]
 fstp  dword [ecx+92]
 fld  dword [edx+100]
 fsub  dword [edx+88]
 fmul  dword [ebx+88]
 fstp  dword [ecx+100]
 fld  dword [edx+96]
 fsub  dword [edx+92]
 fmul  dword [ebx+92]
 fstp  dword [ecx+96]

 fld  dword [ecx]
 fadd  dword [ecx+28]
 fstp  dword [edx]
 fld  dword [ecx]
 fsub  dword [ecx+28]
 fmul  dword [ebx+96]
 fstp  dword [edx+28]
 fld  dword [ecx+4]
 fadd  dword [ecx+24]
 fstp  dword [edx+4]
 fld  dword [ecx+4]
 fsub  dword [ecx+24]
 fmul  dword [ebx+100]
 fstp  dword [edx+24]
 fld  dword [ecx+8]
 fadd  dword [ecx+20]
 fstp  dword [edx+8]
 fld  dword [ecx+8]
 fsub  dword [ecx+20]
 fmul  dword [ebx+104]
 fstp  dword [edx+20]
 fld  dword [ecx+12]
 fadd  dword [ecx+16]
 fstp  dword [edx+12]
 fld  dword [ecx+12]
 fsub  dword [ecx+16]
 fmul  dword [ebx+108]
 fstp  dword [edx+16]
 fld  dword [ecx+32]
 fadd  dword [ecx+60]
 fstp  dword [edx+32]
 fld  dword [ecx+60]
 fsub  dword [ecx+32]
 fmul  dword [ebx+96]
 fstp  dword [edx+60]
 fld  dword [ecx+36]
 fadd  dword [ecx+56]
 fstp  dword [edx+36]
 fld  dword [ecx+56]
 fsub  dword [ecx+36]
 fmul  dword [ebx+100]
 fstp  dword [edx+56]
 fld  dword [ecx+40]
 fadd  dword [ecx+52]
 fstp  dword [edx+40]
 fld  dword [ecx+52]
 fsub  dword [ecx+40]
 fmul  dword [ebx+104]
 fstp  dword [edx+52]
 fld  dword [ecx+44]
 fadd  dword [ecx+48]
 fstp  dword [edx+44]
 fld  dword [ecx+48]
 fsub  dword [ecx+44]
 fmul  dword [ebx+108]
 fstp  dword [edx+48]
 fld  dword [ecx+64]
 fadd  dword [ecx+92]
 fstp  dword [edx+64]
 fld  dword [ecx+64]
 fsub  dword [ecx+92]
 fmul  dword [ebx+96]
 fstp  dword [edx+92]
 fld  dword [ecx+68]
 fadd  dword [ecx+88]
 fstp  dword [edx+68]
 fld  dword [ecx+68]
 fsub  dword [ecx+88]
 fmul  dword [ebx+100]
 fstp  dword [edx+88]
 fld  dword [ecx+72]
 fadd  dword [ecx+84]
 fstp  dword [edx+72]
 fld  dword [ecx+72]
 fsub  dword [ecx+84]
 fmul  dword [ebx+104]
 fstp  dword [edx+84]
 fld  dword [ecx+76]
 fadd  dword [ecx+80]
 fstp  dword [edx+76]
 fld  dword [ecx+76]
 fsub  dword [ecx+80]
 fmul  dword [ebx+108]
 fstp  dword [edx+80]
 fld  dword [ecx+96]
 fadd  dword [ecx+124]
 fstp  dword [edx+96]
 fld  dword [ecx+124]
 fsub  dword [ecx+96]
 fmul  dword [ebx+96]
 fstp  dword [edx+124]
 fld  dword [ecx+100]
 fadd  dword [ecx+120]
 fstp  dword [edx+100]
 fld  dword [ecx+120]
 fsub  dword [ecx+100]
 fmul  dword [ebx+100]
 fstp  dword [edx+120]
 fld  dword [ecx+104]
 fadd  dword [ecx+116]
 fstp  dword [edx+104]
 fld  dword [ecx+116]
 fsub  dword [ecx+104]
 fmul  dword [ebx+104]
 fstp  dword [edx+116]
 fld  dword [ecx+108]
 fadd  dword [ecx+112]
 fstp  dword [edx+108]
 fld  dword [ecx+112]
 fsub  dword [ecx+108]
 fmul  dword [ebx+108]
 fstp  dword [edx+112]
 fld  dword [edx]
 fadd  dword [edx+12]
 fstp  dword [ecx]
 fld  dword [edx]
 fsub  dword [edx+12]
 fmul  dword [ebx+112]
 fstp  dword [ecx+12]
 fld  dword [edx+4]
 fadd  dword [edx+8]
 fstp  dword [ecx+4]
 fld  dword [edx+4]
 fsub  dword [edx+8]
 fmul  dword [ebx+116]
 fstp  dword [ecx+8]
 fld  dword [edx+16]
 fadd  dword [edx+28]
 fstp  dword [ecx+16]
 fld  dword [edx+28]
 fsub  dword [edx+16]
 fmul  dword [ebx+112]
 fstp  dword [ecx+28]
 fld  dword [edx+20]
 fadd  dword [edx+24]
 fstp  dword [ecx+20]
 fld  dword [edx+24]
 fsub  dword [edx+20]
 fmul  dword [ebx+116]
 fstp  dword [ecx+24]
 fld  dword [edx+32]
 fadd  dword [edx+44]
 fstp  dword [ecx+32]
 fld  dword [edx+32]
 fsub  dword [edx+44]
 fmul  dword [ebx+112]
 fstp  dword [ecx+44]
 fld  dword [edx+36]
 fadd  dword [edx+40]
 fstp  dword [ecx+36]
 fld  dword [edx+36]
 fsub  dword [edx+40]
 fmul  dword [ebx+116]
 fstp  dword [ecx+40]
 fld  dword [edx+48]
 fadd  dword [edx+60]
 fstp  dword [ecx+48]
 fld  dword [edx+60]
 fsub  dword [edx+48]
 fmul  dword [ebx+112]
 fstp  dword [ecx+60]
 fld  dword [edx+52]
 fadd  dword [edx+56]
 fstp  dword [ecx+52]
 fld  dword [edx+56]
 fsub  dword [edx+52]
 fmul  dword [ebx+116]
 fstp  dword [ecx+56]
 fld  dword [edx+64]
 fadd  dword [edx+76]
 fstp  dword [ecx+64]
 fld  dword [edx+64]
 fsub  dword [edx+76]
 fmul  dword [ebx+112]
 fstp  dword [ecx+76]
 fld  dword [edx+68]
 fadd  dword [edx+72]
 fstp  dword [ecx+68]
 fld  dword [edx+68]
 fsub  dword [edx+72]
 fmul  dword [ebx+116]
 fstp  dword [ecx+72]
 fld  dword [edx+80]
 fadd  dword [edx+92]
 fstp  dword [ecx+80]
 fld  dword [edx+92]
 fsub  dword [edx+80]
 fmul  dword [ebx+112]
 fstp  dword [ecx+92]
 fld  dword [edx+84]
 fadd  dword [edx+88]
 fstp  dword [ecx+84]
 fld  dword [edx+88]
 fsub  dword [edx+84]
 fmul  dword [ebx+116]
 fstp  dword [ecx+88]
 fld  dword [edx+96]
 fadd  dword [edx+108]
 fstp  dword [ecx+96]
 fld  dword [edx+96]
 fsub  dword [edx+108]
 fmul  dword [ebx+112]
 fstp  dword [ecx+108]
 fld  dword [edx+100]
 fadd  dword [edx+104]
 fstp  dword [ecx+100]
 fld  dword [edx+100]
 fsub  dword [edx+104]
 fmul  dword [ebx+116]
 fstp  dword [ecx+104]
 fld  dword [edx+112]
 fadd  dword [edx+124]
 fstp  dword [ecx+112]
 fld  dword [edx+124]
 fsub  dword [edx+112]
 fmul  dword [ebx+112]
 fstp  dword [ecx+124]
 fld  dword [edx+116]
 fadd  dword [edx+120]
 fstp  dword [ecx+116]
 fld  dword [edx+120]
 fsub  dword [edx+116]
 fmul  dword [ebx+116]
 fstp  dword [ecx+120]

 fld  dword [ecx+32]
 fadd  dword [ecx+36]
 fstp  dword [edx+32]
 fld  dword [ecx+32]
 fsub  dword [ecx+36]
 fmul  dword [ebx+120]
 fstp  dword [edx+36]
 fld  dword [ecx+44]
 fsub  dword [ecx+40]
 fmul  dword [ebx+120]
 fst  dword [edx+44]
 fadd  dword [ecx+40]
 fadd  dword [ecx+44]
 fstp  dword [edx+40]
 fld  dword [ecx+48]
 fsub  dword [ecx+52]
 fmul  dword [ebx+120]
 fld  dword [ecx+60]
 fsub  dword [ecx+56]
 fmul  dword [ebx+120]
 fld st0
 fadd  dword [ecx+56]
 fadd  dword [ecx+60]
 fld st0
 fadd  dword [ecx+48]
 fadd  dword [ecx+52]
 fstp  dword [edx+48]
 fadd st2
 fstp  dword [edx+56]
 fst  dword [edx+60]
 faddp st1
 fstp  dword [edx+52]
 fld  dword [ecx+64]
 fadd  dword [ecx+68]
 fstp  dword [edx+64]
 fld  dword [ecx+64]
 fsub  dword [ecx+68]
 fmul  dword [ebx+120]
 fstp  dword [edx+68]
 fld  dword [ecx+76]
 fsub  dword [ecx+72]
 fmul  dword [ebx+120]
 fst  dword [edx+76]
 fadd  dword [ecx+72]
 fadd  dword [ecx+76]
 fstp  dword [edx+72]
 fld  dword [ecx+92]
 fsub  dword [ecx+88]
 fmul  dword [ebx+120]
 fst  dword [edx+92]
 fadd  dword [ecx+92]
 fadd  dword [ecx+88]
 fld st0
 fadd  dword [ecx+80]
 fadd  dword [ecx+84]
 fstp  dword [edx+80]
 fld  dword [ecx+80]
 fsub  dword [ecx+84]
 fmul  dword [ebx+120]
 fadd st1,st0
 fadd  dword [edx+92]
 fstp  dword [edx+84]
 fstp  dword [edx+88]
 fld  dword [ecx+96]
 fadd  dword [ecx+100]
 fstp  dword [edx+96]
 fld  dword [ecx+96]
 fsub  dword [ecx+100]
 fmul  dword [ebx+120]
 fstp  dword [edx+100]
 fld  dword [ecx+108]
 fsub  dword [ecx+104]
 fmul  dword [ebx+120]
 fst  dword [edx+108]
 fadd  dword [ecx+104]
 fadd  dword [ecx+108]
 fstp  dword [edx+104]
 fld  dword [ecx+124]
 fsub  dword [ecx+120]
 fmul  dword [ebx+120]
 fst  dword [edx+124]
 fadd  dword [ecx+120]
 fadd  dword [ecx+124]
 fld st0
 fadd  dword [ecx+112]
 fadd  dword [ecx+116]
 fstp  dword [edx+112]
 fld  dword [ecx+112]
 fsub  dword [ecx+116]
 fmul  dword [ebx+120]
 fadd st1,st0
 fadd  dword [edx+124]
 fstp  dword [edx+116]
 fstp  dword [edx+120]
 jnz L01

 fld  dword [ecx]
 fadd  dword [ecx+4]
 fstp  dword [esi+1024]
 fld  dword [ecx]
 fsub  dword [ecx+4]
 fmul  dword [ebx+120]
 fst  dword [esi]
 fstp  dword [edi]
 fld  dword [ecx+12]
 fsub  dword [ecx+8]
 fmul  dword [ebx+120]
 fst  dword [edi+512]
 fadd  dword [ecx+12]
 fadd  dword [ecx+8]
 fstp  dword [esi+512]
 fld  dword [ecx+16]
 fsub  dword [ecx+20]
 fmul  dword [ebx+120]
 fld  dword [ecx+28]
 fsub  dword [ecx+24]
 fmul  dword [ebx+120]
 fst  dword [edi+768]
 fld st0
 fadd  dword [ecx+24]
 fadd  dword [ecx+28]
 fld st0
 fadd  dword [ecx+16]
 fadd  dword [ecx+20]
 fstp  dword [esi+768]
 fadd st2
 fstp  dword [esi+256]
 faddp st1
 fstp  dword [edi+256]

 fld  dword [edx+32]
 fadd  dword [edx+48]
 fstp  dword [esi+896]
 fld  dword [edx+48]
 fadd  dword [edx+40]
 fstp  dword [esi+640]
 fld  dword [edx+40]
 fadd  dword [edx+56]
 fstp  dword [esi+384]
 fld  dword [edx+56]
 fadd  dword [edx+36]
 fstp  dword [esi+128]
 fld  dword [edx+36]
 fadd  dword [edx+52]
 fstp  dword [edi+128]
 fld  dword [edx+52]
 fadd  dword [edx+44]
 fstp  dword [edi+384]
 fld  dword [edx+60]
 fst  dword [edi+896]
 fadd  dword [edx+44]
 fstp  dword [edi+640]
 fld  dword [edx+96]
 fadd  dword [edx+112]
 fld st0
 fadd  dword [edx+64]
 fstp  dword [esi+960]
 fadd  dword [edx+80]
 fstp  dword [esi+832]
 fld  dword [edx+112]
 fadd  dword [edx+104]
 fld st0
 fadd  dword [edx+80]
 fstp  dword [esi+704]
 fadd  dword [edx+72]
 fstp  dword [esi+576]
 fld  dword [edx+104]
 fadd  dword [edx+120]
 fld st0
 fadd  dword [edx+72]
 fstp  dword [esi+448]
 fadd  dword [edx+88]
 fstp  dword [esi+320]
 fld  dword [edx+120]
 fadd  dword [edx+100]
 fld st0
 fadd  dword [edx+88]
 fstp  dword [esi+192]
 fadd  dword [edx+68]
 fstp  dword [esi+64]
 fld  dword [edx+100]
 fadd  dword [edx+116]
 fld st0
 fadd  dword [edx+68]
 fstp  dword [edi+64]
 fadd  dword [edx+84]
 fstp  dword [edi+192]
 fld  dword [edx+116]
 fadd  dword [edx+108]
 fld st0
 fadd  dword [edx+84]
 fstp  dword [edi+320]
 fadd  dword [edx+76]
 fstp  dword [edi+448]
 fld  dword [edx+108]
 fadd  dword [edx+124]
 fld st0
 fadd  dword [edx+76]
 fstp  dword [edi+576]
 fadd  dword [edx+92]
 fstp  dword [edi+704]
 fld  dword [edx+124]
 fst  dword [edi+960]
 fadd  dword [edx+92]
 fstp  dword [edi+832]
 add  esp,256
 pop  edi
 pop  esi
 pop  ebx
 ret
L01:
 fld  dword [ecx]
 fadd  dword [ecx+4]
 fistp  dword [esi+512]
 fld  dword [ecx]
 fsub  dword [ecx+4]
 fmul  dword [ebx+120]

 fistp  dword [esi]

 fld  dword [ecx+12]
 fsub  dword [ecx+8]
 fmul  dword [ebx+120]
 fist  dword [edi+256]
 fadd  dword [ecx+12]
 fadd  dword [ecx+8]
 fistp  dword [esi+256]
 fld  dword [ecx+16]
 fsub  dword [ecx+20]
 fmul  dword [ebx+120]
 fld  dword [ecx+28]
 fsub  dword [ecx+24]
 fmul  dword [ebx+120]
 fist  dword [edi+384]
 fld st0
 fadd  dword [ecx+24]
 fadd  dword [ecx+28]
 fld st0
 fadd  dword [ecx+16]
 fadd  dword [ecx+20]
 fistp  dword [esi+384]
 fadd st2
 fistp  dword [esi+128]
 faddp st1
 fistp  dword [edi+128]

 fld  dword [edx+32]
 fadd  dword [edx+48]
 fistp  dword [esi+448]
 fld  dword [edx+48]
 fadd  dword [edx+40]
 fistp  dword [esi+320]
 fld  dword [edx+40]
 fadd  dword [edx+56]
 fistp  dword [esi+192]
 fld  dword [edx+56]
 fadd  dword [edx+36]
 fistp  dword [esi+64]
 fld  dword [edx+36]
 fadd  dword [edx+52]
 fistp  dword [edi+64]
 fld  dword [edx+52]
 fadd  dword [edx+44]
 fistp  dword [edi+192]
 fld  dword [edx+60]
 fist  dword [edi+448]
 fadd  dword [edx+44]
 fistp  dword [edi+320]
 fld  dword [edx+96]
 fadd  dword [edx+112]
 fld st0
 fadd  dword [edx+64]
 fistp  dword [esi+480]
 fadd  dword [edx+80]
 fistp  dword [esi+416]
 fld  dword [edx+112]
 fadd  dword [edx+104]
 fld st0
 fadd  dword [edx+80]
 fistp  dword [esi+352]
 fadd  dword [edx+72]
 fistp  dword [esi+288]
 fld  dword [edx+104]
 fadd  dword [edx+120]
 fld st0
 fadd  dword [edx+72]
 fistp  dword [esi+224]
 fadd  dword [edx+88]
 fistp  dword [esi+160]
 fld  dword [edx+120]
 fadd  dword [edx+100]
 fld st0
 fadd  dword [edx+88]
 fistp  dword [esi+96]
 fadd  dword [edx+68]
 fistp  dword [esi+32]
 fld  dword [edx+100]
 fadd  dword [edx+116]
 fld st0
 fadd  dword [edx+68]
 fistp  dword [edi+32]
 fadd  dword [edx+84]
 fistp  dword [edi+96]
 fld  dword [edx+116]
 fadd  dword [edx+108]
 fld st0
 fadd  dword [edx+84]
 fistp  dword [edi+160]
 fadd  dword [edx+76]
 fistp  dword [edi+224]
 fld  dword [edx+108]
 fadd  dword [edx+124]
 fld st0
 fadd  dword [edx+76]
 fistp  dword [edi+288]
 fadd  dword [edx+92]
 fistp  dword [edi+352]
 fld  dword [edx+124]
 fist  dword [edi+480]
 fadd  dword [edx+92]
 fistp  dword [edi+416]
 movsw
 add  esp,256
 pop  edi
 pop  esi
 pop  ebx
 ret
; 825 "dct64_mmx.S"


