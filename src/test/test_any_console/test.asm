;测试函数   三个数相加  
;.386
.model flat, c
;public test_

.code

sub_1025E453    proc near

var_8           = dword ptr -8
anonymous_0     = word ptr  8
	push    ebx
	mov     ebx, esp
	push    ecx
	push    ecx
	and     esp, 0FFFFFFF0h
	add     esp, 4
	push    ebp
	mov     ebp, [ebx + 4]
	mov[esp + 0Ch + var_8], ebp
	mov     ebp, esp
	sub     esp, 28h
	push    esi
	push    edi
	mov     edi, [ebx + 0Ch]
	xor eax, eax
	movzx   ecx, word ptr[edi]
	mov[ebp - 8], ecx
	cmp     ax, cx
	jnz     short loc_1025E485
	mov     eax, [ebx + 8]
	jmp     loc_1025E6C6

	loc_1025E485 :
		;cmp     0, 1
		;jmp     loc_1025E57C
		mov     eax, edi
		and     eax, 0FFFh
		push    2
		pop     esi
		cmp     eax, 0FF0h
		ja      short loc_1025E4A8
		movups  xmm0, xmmword ptr[edi]
		jmp     short loc_1025E4D3

	loc_1025E4A8 :
		push    8
		pxor    xmm0, xmm0
		movzx   ecx, cx
		pop     edx

	loc_1025E4B2 :
		movzx   eax, cx
		psrldq  xmm0, 2
		pinsrw  xmm0, eax, 7
		xor eax, eax
		cmp     ax, cx
		jz      short loc_1025E4CB
		add     edi, esi
		movzx   ecx, word ptr[edi]

	loc_1025E4CB :
		sub     edx, 1
		jnz     short loc_1025E4B2
		mov     ecx, [ebp - 8]

	loc_1025E4D3 :
		mov     edx, [ebx + 8]

	loc_1025E4D6 :
		mov     eax, edx
		and     eax, 0FFFh
		cmp     eax, 0FF0h
		ja      short loc_1025E53F
		pcmpistri xmm0, xmmword ptr[edx], 0Dh
		jbe     short loc_1025E4F4
		add     edx, 10h

	loc_1025E4EF :
		mov     ecx, [ebp - 8]
		jmp     short loc_1025E4D6

	loc_1025E4F4 :
		jnb     loc_1025E6C4
		pcmpistri xmm0, xmmword ptr[edx], 0Dh
		lea     edx, [edx + ecx * 2]

	loc_1025E503 :
		mov     ecx, [ebx + 0Ch]
		mov     edi, edx

	loc_1025E508 :
		mov     eax, edi
		mov[ebp - 4], ecx
		and     eax, 0FFFh
		cmp     eax, 0FF0h
		ja      short loc_1025E556
		mov     eax, ecx
		and     eax, 0FFFh
		cmp     eax, 0FF0h
		ja      short loc_1025E556
		movups  xmm1, xmmword ptr[ecx]
		pcmpistri xmm1, xmmword ptr[edi], 0Dh
		jno     short loc_1025E56E
		js      short loc_1025E575
		mov     ecx, [ebp - 4]
		add     edi, 10h
		add     ecx, 10h
		jmp     short loc_1025E508

	loc_1025E53F :
		movzx   eax, word ptr[edx]
		xor edi, edi
		cmp     di, ax
		jz      loc_1025E6C4
		cmp     ax, cx
		jz      short loc_1025E503
		add     edx, esi
		jmp     short loc_1025E4D6

	loc_1025E556 :
		movzx   eax, word ptr[ecx]
		and dword ptr[ebp - 4], 0
		cmp[ebp - 4], ax
		jz      short loc_1025E575
		cmp[edi], ax
		jnz     short loc_1025E56E
		add     edi, esi
		add     ecx, esi
		jmp     short loc_1025E508

	loc_1025E56E :
		add     edx, esi
		jmp     loc_1025E4EF

	loc_1025E575 :
		mov     eax, edx
		jmp     loc_1025E6C6

	loc_1025E57C :
		jnz     loc_1025E65F
		movzx   eax, cx
		pxor    xmm3, xmm3
		mov     ecx, [ebx + 8]
		push    2
		pop     esi
		movd    xmm0, eax
		pshuflw xmm0, xmm0, 0
		pshufd  xmm4, xmm0, 0

	loc_1025E59D :
		mov     eax, ecx
		and     eax, 0FFFh
		cmp     eax, 0FF0h
		ja      short loc_1025E5D2
		movups  xmm1, xmmword ptr[ecx]
		movaps  xmm0, xmm3
		pcmpeqw xmm0, xmm1
		pcmpeqw xmm1, xmm4
		por     xmm0, xmm1
		pmovmskb eax, xmm0
		test    eax, eax
		jnz     short loc_1025E5CA
		add     ecx, 10h
		jmp     short loc_1025E59D

	loc_1025E5CA :
		bsf     eax, eax
		shr     eax, 1
		lea     ecx, [ecx + eax * 2]

	loc_1025E5D2 :
		movzx   eax, word ptr[ecx]
		xor edx, edx
		cmp     dx, ax
		jz      loc_1025E6C4
		movzx   edx, word ptr[edi]
		cmp     dx, ax
		jnz     short loc_1025E654
		mov     edx, ecx

	loc_1025E5EA :
		mov     eax, edi
		and     eax, 0FFFh
		cmp     eax, 0FF0h
		ja      short loc_1025E639
		mov     eax, edx
		and     eax, 0FFFh
		cmp     eax, 0FF0h
		ja      short loc_1025E639
		movups  xmm2, xmmword ptr[edi]
		movups  xmm1, xmmword ptr[edx]
		movaps  xmm0, xmm3
		pcmpeqw xmm1, xmm2
		pcmpeqw xmm0, xmm2
		pcmpeqw xmm1, xmm3
		por     xmm1, xmm0
		pmovmskb eax, xmm1
		test    eax, eax
		jnz     short loc_1025E62F
		add     edx, 10h
		add     edi, 10h
		jmp     short loc_1025E5EA

	loc_1025E62F :
		bsf     eax, eax
		and     eax, 0FFFFFFFEh
		add     edx, eax
		add     edi, eax

	loc_1025E639 :
		movzx   eax, word ptr[edi]
		and dword ptr[ebp - 8], 0
		cmp[ebp - 8], ax
		jz      short loc_1025E65B
		cmp[edx], ax
		jnz     short loc_1025E651
		add     edx, esi
		add     edi, esi
		jmp     short loc_1025E5EA

	loc_1025E651 :
		mov     edi, [ebx + 0Ch]

	loc_1025E654 :
		add     ecx, esi
		jmp     loc_1025E59D

	loc_1025E65B :
		mov     eax, ecx
		jmp     short loc_1025E6C6

	loc_1025E65F :
		mov     edx, [ebx + 8]
		movzx   eax, word ptr[edx]
		test    ax, ax
		jz      short loc_1025E6C4
		and     dword ptr[ebp - 8], 0
		mov     ecx, eax
		mov     eax, edx
		sub     eax, edi
		mov[ebp - 4], eax
		push    2
		pop     esi
		test    cx, cx
		jz      short loc_1025E6A6

	loc_1025E67F :
		movzx   ecx, word ptr[edi]
		test    cx, cx
		jz      loc_1025E575
		movzx   ecx, word ptr[eax + edi]
		movzx   eax, word ptr[edi]
		sub     ecx, eax
		mov     eax, [ebp - 4]
		mov     ecx, [ebp - 8]
		jnz     short loc_1025E6A9
		add     edi, esi
		cmp[eax + edi], cx
		jnz     short loc_1025E67F
		jmp     short loc_1025E6A9

	loc_1025E6A6 :
		mov     ecx, [ebp - 8]

	loc_1025E6A9 :
		cmp[edi], cx
		jz      loc_1025E575
		mov     edi, [ebx + 0Ch]
		add     edx, esi
		add     eax, esi
		mov[ebp - 4], eax
		movzx   ecx, word ptr[edx]
		test    cx, cx
		jnz     short loc_1025E67F

	loc_1025E6C4 :
		xor     eax, eax

	loc_1025E6C6 :
		pop     edi
		pop     esi
		mov     esp, ebp
		pop     ebp
		mov     esp, ebx
		pop     ebx
		retn
sub_1025E453    endp
end