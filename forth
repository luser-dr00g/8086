
8086	Forth
SI	IP
SP	Data Stack
BP	stack indexing
DI	Return Stack


macro next
	lodsl		ad
	jmp *(eax)      ff\0350 

macro pushrsp reg
	lea -4(ebp),ebp		8d\177\374
	movl \reg,(ebp)		8b\0?7

macro poprsp reg
	mov (ebp),\reg		89\0?7
	lea 4(ebp),ebp		8d\177\4

docol
	pushrsp esi		pushrsp(6)
	addl $4,eax		05\4\0
	movl eax,esi		8b\370
	next

drop
	pop eax		58
	next

swap
	pop eax		58
	pop ebx		5b
	push eax	50
	push ebx	53
	next

dup
	mov (esp),eax	8b\364  8b\106\0
	push eax	50
	next

over
	mov 4(esp),eax	8b\364  8b\106\2
	push eax	50
	next

rot
	pop eax		58
	pop ebx		5b
	pop ecx		59
	push ebx	53
	push eax	50
	push ecx	51
	next

-rot
	pop eax		58
	pop ebx		5b
	pop ecx		59
	push eax	50
	push ecx	51
	push ebx	53
	next

2drop
	pop eax		58
	pop eax		58
	next

2dup
	mov (esp),eax	8b\364  8b\106\0
	mov 4(esp),ebx          8b\136\2
	push ebx	53
	push eax	50
	next

2swap
	pop eax		58
	pop ebx		5b
	pop ecx		59
	pop edx		5a
	push ebx	53
	push eax	50
	push edx	52
	push ecx	51
	next

?dup
	movl (esp),eax	8b\364	8b\106\0
	test eax,eax	85\300
	jz 1f		74\1
	push eax	58
1
	next


1+
	incl (esp)	8b\364  ff\106\0
	next

1-
	decl (esp)	8b\364  ff\116\0
	next

+
	pop eax		58
	add eax,(esp)	8b\364  01\106\0
	next

-
	pop eax		58
	sub eax,(esp)	8b\364  29\106\0
	next

*
	pop eax		58
	pop ebx		5b
	imull ebx,eax	f7\353
	push eax	50
	next
