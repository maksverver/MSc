	.file	"SmallProgressMeasures.cc"
	.section	.text._ZN16ParityGameSolverD1Ev,"axG",@progbits,_ZN16ParityGameSolverD1Ev,comdat
	.align 2
	.p2align 4,,15
	.weak	_ZN16ParityGameSolverD1Ev
	.type	_ZN16ParityGameSolverD1Ev, @function
_ZN16ParityGameSolverD1Ev:
.LFB983:
	pushl	%ebp
.LCFI0:
	movl	%esp, %ebp
.LCFI1:
	movl	8(%ebp), %eax
	movl	$_ZTV16ParityGameSolver+8, (%eax)
	popl	%ebp
	ret
.LFE983:
	.size	_ZN16ParityGameSolverD1Ev, .-_ZN16ParityGameSolverD1Ev
	.text
	.align 2
	.p2align 4,,15
.globl _ZN21SmallProgressMeasures4liftEj
	.type	_ZN21SmallProgressMeasures4liftEj, @function
_ZN21SmallProgressMeasures4liftEj:
.LFB1006:
	pushl	%ebp
.LCFI2:
	movl	%esp, %ebp
.LCFI3:
	popl	%ebp
	ret
.LFE1006:
	.size	_ZN21SmallProgressMeasures4liftEj, .-_ZN21SmallProgressMeasures4liftEj
	.align 2
	.p2align 4,,15
.globl _ZN21SmallProgressMeasures5solveEv
	.type	_ZN21SmallProgressMeasures5solveEv, @function
_ZN21SmallProgressMeasures5solveEv:
.LFB1007:
	pushl	%ebp
.LCFI4:
	movl	$-1, %ecx
	movl	%esp, %ebp
.LCFI5:
	pushl	%ebx
.LCFI6:
	subl	$20, %esp
.LCFI7:
	movl	8(%ebp), %ebx
	.p2align 4,,7
	.p2align 3
.L7:
	movl	8(%ebx), %eax
	movl	(%eax), %edx
	movl	%ecx, 4(%esp)
	movl	$0, 8(%esp)
	movl	%eax, (%esp)
	call	*8(%edx)
	cmpl	$-1, %eax
	movl	%eax, %ecx
	jne	.L7
	addl	$20, %esp
	movl	$1, %eax
	popl	%ebx
	popl	%ebp
	ret
.LFE1007:
	.size	_ZN21SmallProgressMeasures5solveEv, .-_ZN21SmallProgressMeasures5solveEv
	.align 2
	.p2align 4,,15
.globl _ZN21SmallProgressMeasures6winnerEj
	.type	_ZN21SmallProgressMeasures6winnerEj, @function
_ZN21SmallProgressMeasures6winnerEj:
.LFB1008:
	pushl	%ebp
.LCFI8:
	movl	%esp, %ebp
.LCFI9:
	movl	8(%ebp), %edx
	movl	12(%edx), %eax
	movl	20(%edx), %edx
	sall	$2, %eax
	imull	12(%ebp), %eax
	popl	%ebp
	cmpl	$-1, (%edx,%eax)
	sete	%al
	movzbl	%al, %eax
	ret
.LFE1008:
	.size	_ZN21SmallProgressMeasures6winnerEj, .-_ZN21SmallProgressMeasures6winnerEj
	.p2align 4,,15
	.type	_GLOBAL__I__ZN21SmallProgressMeasuresC2ERK10ParityGameR15LiftingStrategy, @function
_GLOBAL__I__ZN21SmallProgressMeasuresC2ERK10ParityGameR15LiftingStrategy:
.LFB1010:
	pushl	%ebp
.LCFI10:
	movl	%esp, %ebp
.LCFI11:
	subl	$24, %esp
.LCFI12:
	movl	$_ZStL8__ioinit, (%esp)
	call	_ZNSt8ios_base4InitC1Ev
	movl	$__dso_handle, 8(%esp)
	movl	$_ZStL8__ioinit, 4(%esp)
	movl	$_ZNSt8ios_base4InitD1Ev, (%esp)
	call	__cxa_atexit
	leave
	ret
.LFE1010:
	.size	_GLOBAL__I__ZN21SmallProgressMeasuresC2ERK10ParityGameR15LiftingStrategy, .-_GLOBAL__I__ZN21SmallProgressMeasuresC2ERK10ParityGameR15LiftingStrategy
	.section	.ctors,"aw",@progbits
	.align 4
	.long	_GLOBAL__I__ZN21SmallProgressMeasuresC2ERK10ParityGameR15LiftingStrategy
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"SmallProgressMeasures.cc"
.LC1:
	.string	"it != end"
	.section	.text._ZN21SmallProgressMeasures12get_ext_succEjb,"axG",@progbits,_ZN21SmallProgressMeasures12get_ext_succEjb,comdat
	.align 2
	.p2align 4,,15
	.weak	_ZN21SmallProgressMeasures12get_ext_succEjb
	.type	_ZN21SmallProgressMeasures12get_ext_succEjb, @function
_ZN21SmallProgressMeasures12get_ext_succEjb:
.LFB1003:
	pushl	%ebp
.LCFI13:
	movl	%esp, %ebp
.LCFI14:
	pushl	%edi
.LCFI15:
	pushl	%esi
.LCFI16:
	pushl	%ebx
.LCFI17:
	subl	$44, %esp
.LCFI18:
	movl	8(%ebp), %edx
	movzbl	16(%ebp), %eax
	movl	12(%ebp), %ebx
	movl	12(%ebp), %edi
	movl	4(%edx), %esi
	movb	%al, -33(%ebp)
	movl	20(%esi), %edx
	movl	12(%esi), %ecx
	movl	(%edx,%ebx,4), %eax
	leal	(%ecx,%eax,4), %ebx
	movl	4(%edx,%edi,4), %eax
	leal	(%ecx,%eax,4), %eax
	cmpl	%eax, %ebx
	movl	%eax, -32(%ebp)
	je	.L30
	movl	(%ebx), %eax
	leal	4(%ebx), %edi
	cmpl	-32(%ebp), %edi
	movl	%eax, -28(%ebp)
	je	.L17
	movl	32(%esi), %eax
	movl	12(%ebp), %edx
	movl	8(%ebp), %ebx
	movzbl	1(%eax,%edx,2), %esi
	movl	8(%ebp), %edx
	movl	20(%ebx), %ebx
	sarl	%esi
	movl	12(%edx), %eax
	movl	%ebx, -24(%ebp)
	sall	$2, %eax
	movl	%eax, -20(%ebp)
	.p2align 4,,7
	.p2align 3
.L27:
	movl	(%edi), %ebx
	testl	%esi, %esi
	movl	%ebx, -16(%ebp)
	je	.L18
	movl	-20(%ebp), %eax
	movl	-24(%ebp), %ecx
	imull	%ebx, %eax
	movl	-24(%ebp), %ebx
	addl	%eax, %ebx
	movl	-28(%ebp), %eax
	imull	-20(%ebp), %eax
	addl	%eax, %ecx
	movl	(%ebx), %eax
	cmpl	%eax, (%ecx)
	jb	.L19
	ja	.L20
	xorl	%edx, %edx
	jmp	.L21
	.p2align 4,,7
	.p2align 3
.L22:
	movl	(%ebx,%edx,4), %eax
	cmpl	%eax, (%ecx,%edx,4)
	.p2align 4,,5
	.p2align 3
	jb	.L19
	.p2align 4,,5
	.p2align 3
	ja	.L20
.L21:
	addl	$1, %edx
	cmpl	%edx, %esi
	.p2align 4,,3
	.p2align 3
	jg	.L22
.L18:
	xorl	%eax, %eax
.L23:
	cmpb	$0, -33(%ebp)
	.p2align 4,,3
	.p2align 3
	je	.L24
	testl	%eax, %eax
	setg	%al
.L25:
	testb	%al, %al
	je	.L26
	movl	-16(%ebp), %eax
	movl	%eax, -28(%ebp)
.L26:
	addl	$4, %edi
	cmpl	-32(%ebp), %edi
	jne	.L27
.L17:
	movl	-28(%ebp), %eax
	addl	$44, %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.p2align 4,,7
	.p2align 3
.L24:
	shrl	$31, %eax
	jmp	.L25
	.p2align 4,,7
	.p2align 3
.L19:
	movl	$-1, %eax
	jmp	.L23
	.p2align 4,,7
	.p2align 3
.L20:
	movl	$1, %eax
	.p2align 4,,5
	.p2align 3
	jmp	.L23
.L30:
	movl	$_ZZN21SmallProgressMeasures12get_ext_succEjbE19__PRETTY_FUNCTION__, 12(%esp)
	movl	$42, 8(%esp)
	movl	$.LC0, 4(%esp)
	movl	$.LC1, (%esp)
	call	__assert_fail
.LFE1003:
	.size	_ZN21SmallProgressMeasures12get_ext_succEjb, .-_ZN21SmallProgressMeasures12get_ext_succEjb
	.text
	.align 2
	.p2align 4,,15
.globl _ZN21SmallProgressMeasuresD1Ev
	.type	_ZN21SmallProgressMeasuresD1Ev, @function
_ZN21SmallProgressMeasuresD1Ev:
.LFB1000:
	pushl	%ebp
.LCFI19:
	movl	%esp, %ebp
.LCFI20:
	pushl	%ebx
.LCFI21:
	subl	$4, %esp
.LCFI22:
	movl	8(%ebp), %ebx
	movl	20(%ebx), %eax
	movl	$_ZTV21SmallProgressMeasures+8, (%ebx)
	testl	%eax, %eax
	je	.L32
	movl	%eax, (%esp)
	call	_ZdaPv
.L32:
	movl	16(%ebx), %eax
	testl	%eax, %eax
	je	.L33
	movl	%eax, (%esp)
	call	_ZdaPv
.L33:
	movl	$_ZTV16ParityGameSolver+8, (%ebx)
	addl	$4, %esp
	popl	%ebx
	popl	%ebp
	ret
.LFE1000:
	.size	_ZN21SmallProgressMeasuresD1Ev, .-_ZN21SmallProgressMeasuresD1Ev
	.align 2
	.p2align 4,,15
.globl _ZN21SmallProgressMeasuresD2Ev
	.type	_ZN21SmallProgressMeasuresD2Ev, @function
_ZN21SmallProgressMeasuresD2Ev:
.LFB999:
	pushl	%ebp
.LCFI23:
	movl	%esp, %ebp
.LCFI24:
	pushl	%ebx
.LCFI25:
	subl	$4, %esp
.LCFI26:
	movl	8(%ebp), %ebx
	movl	20(%ebx), %eax
	movl	$_ZTV21SmallProgressMeasures+8, (%ebx)
	testl	%eax, %eax
	je	.L37
	movl	%eax, (%esp)
	call	_ZdaPv
.L37:
	movl	16(%ebx), %eax
	testl	%eax, %eax
	je	.L38
	movl	%eax, (%esp)
	call	_ZdaPv
.L38:
	movl	$_ZTV16ParityGameSolver+8, (%ebx)
	addl	$4, %esp
	popl	%ebx
	popl	%ebp
	ret
.LFE999:
	.size	_ZN21SmallProgressMeasuresD2Ev, .-_ZN21SmallProgressMeasuresD2Ev
	.align 2
	.p2align 4,,15
.globl _ZN21SmallProgressMeasuresD0Ev
	.type	_ZN21SmallProgressMeasuresD0Ev, @function
_ZN21SmallProgressMeasuresD0Ev:
.LFB1001:
	pushl	%ebp
.LCFI27:
	movl	%esp, %ebp
.LCFI28:
	pushl	%ebx
.LCFI29:
	subl	$4, %esp
.LCFI30:
	movl	8(%ebp), %ebx
	movl	20(%ebx), %eax
	movl	$_ZTV21SmallProgressMeasures+8, (%ebx)
	testl	%eax, %eax
	je	.L42
	movl	%eax, (%esp)
	call	_ZdaPv
.L42:
	movl	16(%ebx), %eax
	testl	%eax, %eax
	je	.L43
	movl	%eax, (%esp)
	call	_ZdaPv
.L43:
	movl	$_ZTV16ParityGameSolver+8, (%ebx)
	movl	%ebx, 8(%ebp)
	addl	$4, %esp
	popl	%ebx
	popl	%ebp
	jmp	_ZdlPv
.LFE1001:
	.size	_ZN21SmallProgressMeasuresD0Ev, .-_ZN21SmallProgressMeasuresD0Ev
	.section	.text._ZN16ParityGameSolverD0Ev,"axG",@progbits,_ZN16ParityGameSolverD0Ev,comdat
	.align 2
	.p2align 4,,15
	.weak	_ZN16ParityGameSolverD0Ev
	.type	_ZN16ParityGameSolverD0Ev, @function
_ZN16ParityGameSolverD0Ev:
.LFB984:
	pushl	%ebp
.LCFI31:
	movl	%esp, %ebp
.LCFI32:
	movl	8(%ebp), %eax
	movl	$_ZTV16ParityGameSolver+8, (%eax)
	popl	%ebp
	jmp	_ZdlPv
.LFE984:
	.size	_ZN16ParityGameSolverD0Ev, .-_ZN16ParityGameSolverD0Ev
.globl _Unwind_Resume
	.text
	.align 2
	.p2align 4,,15
.globl _ZN21SmallProgressMeasuresC1ERK10ParityGameR15LiftingStrategy
	.type	_ZN21SmallProgressMeasuresC1ERK10ParityGameR15LiftingStrategy, @function
_ZN21SmallProgressMeasuresC1ERK10ParityGameR15LiftingStrategy:
.LFB997:
	pushl	%ebp
.LCFI33:
	movl	%esp, %ebp
.LCFI34:
	pushl	%edi
.LCFI35:
	pushl	%esi
.LCFI36:
	pushl	%ebx
.LCFI37:
	subl	$28, %esp
.LCFI38:
	movl	12(%ebp), %edi
	movl	8(%ebp), %ebx
	movl	16(%ebp), %eax
	movl	(%edi), %edx
	movl	%edi, 4(%ebx)
	movl	%eax, 8(%ebx)
	movl	$_ZTV21SmallProgressMeasures+8, (%ebx)
	movl	%edx, %eax
	shrl	$31, %eax
	addl	%edx, %eax
	sarl	%eax
	movl	%eax, 12(%ebx)
	sall	$2, %eax
	movl	%eax, (%esp)
.LEHB0:
	call	_Znaj
	movl	12(%ebx), %ecx
	movl	%eax, 16(%ebx)
	testl	%ecx, %ecx
	jle	.L50
	movl	4(%ebx), %edx
	movl	%eax, %esi
	.p2align 4,,7
	.p2align 3
.L51:
	movl	36(%edx), %eax
	movl	4(%eax), %eax
	movl	%eax, (%esi)
	movl	12(%ebx), %eax
	leal	1(%eax), %ecx
	testl	%ecx, %ecx
	movl	%ecx, 12(%ebx)
	jg	.L51
.L50:
	movl	4(%edi), %eax
	leal	0(,%eax,4), %esi
	imull	%ecx, %esi
	movl	%esi, (%esp)
	call	_Znaj
.LEHE0:
	movl	%eax, 20(%ebx)
	movl	%esi, 16(%ebp)
	movl	$0, 12(%ebp)
	movl	%eax, 8(%ebp)
	addl	$28, %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	jmp	memset
.L54:
.L52:
	movl	$_ZTV16ParityGameSolver+8, (%ebx)
	movl	%eax, (%esp)
.LEHB1:
	call	_Unwind_Resume
.LEHE1:
.LFE997:
	.size	_ZN21SmallProgressMeasuresC1ERK10ParityGameR15LiftingStrategy, .-_ZN21SmallProgressMeasuresC1ERK10ParityGameR15LiftingStrategy
.globl __gxx_personality_v0
	.section	.gcc_except_table,"a",@progbits
.LLSDA997:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE997-.LLSDACSB997
.LLSDACSB997:
	.uleb128 .LEHB0-.LFB997
	.uleb128 .LEHE0-.LEHB0
	.uleb128 .L54-.LFB997
	.uleb128 0x0
	.uleb128 .LEHB1-.LFB997
	.uleb128 .LEHE1-.LEHB1
	.uleb128 0x0
	.uleb128 0x0
.LLSDACSE997:
	.text
	.align 2
	.p2align 4,,15
.globl _ZN21SmallProgressMeasuresC2ERK10ParityGameR15LiftingStrategy
	.type	_ZN21SmallProgressMeasuresC2ERK10ParityGameR15LiftingStrategy, @function
_ZN21SmallProgressMeasuresC2ERK10ParityGameR15LiftingStrategy:
.LFB996:
	pushl	%ebp
.LCFI39:
	movl	%esp, %ebp
.LCFI40:
	pushl	%edi
.LCFI41:
	pushl	%esi
.LCFI42:
	pushl	%ebx
.LCFI43:
	subl	$28, %esp
.LCFI44:
	movl	12(%ebp), %edi
	movl	8(%ebp), %ebx
	movl	16(%ebp), %eax
	movl	(%edi), %edx
	movl	%edi, 4(%ebx)
	movl	%eax, 8(%ebx)
	movl	$_ZTV21SmallProgressMeasures+8, (%ebx)
	movl	%edx, %eax
	shrl	$31, %eax
	addl	%edx, %eax
	sarl	%eax
	movl	%eax, 12(%ebx)
	sall	$2, %eax
	movl	%eax, (%esp)
.LEHB2:
	call	_Znaj
	movl	12(%ebx), %ecx
	movl	%eax, 16(%ebx)
	testl	%ecx, %ecx
	jle	.L57
	movl	4(%ebx), %edx
	movl	%eax, %esi
	.p2align 4,,7
	.p2align 3
.L58:
	movl	36(%edx), %eax
	movl	4(%eax), %eax
	movl	%eax, (%esi)
	movl	12(%ebx), %eax
	leal	1(%eax), %ecx
	testl	%ecx, %ecx
	movl	%ecx, 12(%ebx)
	jg	.L58
.L57:
	movl	4(%edi), %eax
	leal	0(,%eax,4), %esi
	imull	%ecx, %esi
	movl	%esi, (%esp)
	call	_Znaj
.LEHE2:
	movl	%eax, 20(%ebx)
	movl	%esi, 16(%ebp)
	movl	$0, 12(%ebp)
	movl	%eax, 8(%ebp)
	addl	$28, %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	jmp	memset
.L61:
.L59:
	movl	$_ZTV16ParityGameSolver+8, (%ebx)
	movl	%eax, (%esp)
.LEHB3:
	call	_Unwind_Resume
.LEHE3:
.LFE996:
	.size	_ZN21SmallProgressMeasuresC2ERK10ParityGameR15LiftingStrategy, .-_ZN21SmallProgressMeasuresC2ERK10ParityGameR15LiftingStrategy
	.section	.gcc_except_table
.LLSDA996:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE996-.LLSDACSB996
.LLSDACSB996:
	.uleb128 .LEHB2-.LFB996
	.uleb128 .LEHE2-.LEHB2
	.uleb128 .L61-.LFB996
	.uleb128 0x0
	.uleb128 .LEHB3-.LFB996
	.uleb128 .LEHE3-.LEHB3
	.uleb128 0x0
	.uleb128 0x0
.LLSDACSE996:
	.text
	.align 2
	.p2align 4,,15
.globl _ZN21SmallProgressMeasures12get_min_succEj
	.type	_ZN21SmallProgressMeasures12get_min_succEj, @function
_ZN21SmallProgressMeasures12get_min_succEj:
.LFB1004:
	pushl	%ebp
.LCFI45:
	movl	%esp, %ebp
.LCFI46:
	pushl	%edi
.LCFI47:
	pushl	%esi
.LCFI48:
	pushl	%ebx
.LCFI49:
	subl	$44, %esp
.LCFI50:
	movl	8(%ebp), %eax
	movl	12(%ebp), %ebx
	movl	12(%ebp), %edi
	movl	4(%eax), %esi
	movl	20(%esi), %edx
	movl	12(%esi), %ecx
	movl	(%edx,%ebx,4), %eax
	leal	(%ecx,%eax,4), %ebx
	movl	4(%edx,%edi,4), %eax
	leal	(%ecx,%eax,4), %eax
	cmpl	%eax, %ebx
	movl	%eax, -32(%ebp)
	je	.L73
	movl	(%ebx), %eax
	leal	4(%ebx), %edi
	cmpl	%edi, -32(%ebp)
	movl	%eax, -28(%ebp)
	je	.L65
	movl	32(%esi), %eax
	movl	12(%ebp), %edx
	movzbl	1(%eax,%edx,2), %ebx
	movl	8(%ebp), %eax
	movl	8(%ebp), %edx
	sarl	%ebx
	movl	20(%eax), %eax
	movl	%eax, -24(%ebp)
	movl	12(%edx), %eax
	sall	$2, %eax
	movl	%eax, -20(%ebp)
	.p2align 4,,7
	.p2align 3
.L70:
	movl	(%edi), %eax
	testl	%ebx, %ebx
	movl	%eax, -16(%ebp)
	je	.L66
	movl	-20(%ebp), %eax
	imull	-16(%ebp), %eax
	movl	-24(%ebp), %esi
	movl	-24(%ebp), %ecx
	addl	%eax, %esi
	movl	-28(%ebp), %eax
	imull	-20(%ebp), %eax
	addl	%eax, %ecx
	movl	(%esi), %eax
	cmpl	%eax, (%ecx)
	jb	.L67
	ja	.L66
	xorl	%edx, %edx
	jmp	.L68
	.p2align 4,,7
	.p2align 3
.L69:
	movl	(%esi,%edx,4), %eax
	cmpl	%eax, (%ecx,%edx,4)
	.p2align 4,,5
	.p2align 3
	jb	.L67
	.p2align 4,,5
	.p2align 3
	ja	.L66
.L68:
	addl	$1, %edx
	cmpl	%edx, %ebx
	.p2align 4,,3
	.p2align 3
	jg	.L69
.L66:
	addl	$4, %edi
	cmpl	%edi, -32(%ebp)
	.p2align 4,,3
	.p2align 3
	jne	.L70
.L65:
	movl	-28(%ebp), %eax
	addl	$44, %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.p2align 4,,7
	.p2align 3
.L67:
	movl	-16(%ebp), %edx
	addl	$4, %edi
	cmpl	%edi, -32(%ebp)
	movl	%edx, -28(%ebp)
	jne	.L70
	jmp	.L65
.L73:
	movl	$_ZZN21SmallProgressMeasures12get_ext_succEjbE19__PRETTY_FUNCTION__, 12(%esp)
	movl	$42, 8(%esp)
	movl	$.LC0, 4(%esp)
	movl	$.LC1, (%esp)
	call	__assert_fail
.LFE1004:
	.size	_ZN21SmallProgressMeasures12get_min_succEj, .-_ZN21SmallProgressMeasures12get_min_succEj
	.align 2
	.p2align 4,,15
.globl _ZN21SmallProgressMeasures12get_max_succEj
	.type	_ZN21SmallProgressMeasures12get_max_succEj, @function
_ZN21SmallProgressMeasures12get_max_succEj:
.LFB1005:
	pushl	%ebp
.LCFI51:
	movl	%esp, %ebp
.LCFI52:
	pushl	%edi
.LCFI53:
	pushl	%esi
.LCFI54:
	pushl	%ebx
.LCFI55:
	subl	$44, %esp
.LCFI56:
	movl	8(%ebp), %eax
	movl	12(%ebp), %ebx
	movl	12(%ebp), %edi
	movl	4(%eax), %esi
	movl	20(%esi), %edx
	movl	12(%esi), %ecx
	movl	(%edx,%ebx,4), %eax
	leal	(%ecx,%eax,4), %ebx
	movl	4(%edx,%edi,4), %eax
	leal	(%ecx,%eax,4), %eax
	cmpl	%eax, %ebx
	movl	%eax, -32(%ebp)
	je	.L84
	movl	(%ebx), %eax
	leal	4(%ebx), %edi
	cmpl	%edi, -32(%ebp)
	movl	%eax, -28(%ebp)
	je	.L76
	movl	32(%esi), %eax
	movl	12(%ebp), %edx
	movzbl	1(%eax,%edx,2), %ebx
	movl	8(%ebp), %eax
	movl	8(%ebp), %edx
	sarl	%ebx
	movl	20(%eax), %eax
	movl	%eax, -24(%ebp)
	movl	12(%edx), %eax
	sall	$2, %eax
	movl	%eax, -20(%ebp)
	.p2align 4,,7
	.p2align 3
.L81:
	movl	(%edi), %eax
	testl	%ebx, %ebx
	movl	%eax, -16(%ebp)
	je	.L77
	movl	-20(%ebp), %eax
	imull	-16(%ebp), %eax
	movl	-24(%ebp), %esi
	movl	-24(%ebp), %ecx
	addl	%eax, %esi
	movl	-28(%ebp), %eax
	imull	-20(%ebp), %eax
	addl	%eax, %ecx
	movl	(%esi), %eax
	cmpl	%eax, (%ecx)
	jb	.L77
	ja	.L78
	xorl	%edx, %edx
	jmp	.L79
	.p2align 4,,7
	.p2align 3
.L80:
	movl	(%esi,%edx,4), %eax
	cmpl	%eax, (%ecx,%edx,4)
	.p2align 4,,5
	.p2align 3
	jb	.L77
	.p2align 4,,5
	.p2align 3
	ja	.L78
.L79:
	addl	$1, %edx
	cmpl	%edx, %ebx
	.p2align 4,,3
	.p2align 3
	jg	.L80
.L77:
	addl	$4, %edi
	cmpl	%edi, -32(%ebp)
	.p2align 4,,3
	.p2align 3
	jne	.L81
.L76:
	movl	-28(%ebp), %eax
	addl	$44, %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.p2align 4,,7
	.p2align 3
.L78:
	movl	-16(%ebp), %edx
	addl	$4, %edi
	cmpl	%edi, -32(%ebp)
	movl	%edx, -28(%ebp)
	jne	.L81
	jmp	.L76
.L84:
	movl	$_ZZN21SmallProgressMeasures12get_ext_succEjbE19__PRETTY_FUNCTION__, 12(%esp)
	movl	$42, 8(%esp)
	movl	$.LC0, 4(%esp)
	movl	$.LC1, (%esp)
	call	__assert_fail
.LFE1005:
	.size	_ZN21SmallProgressMeasures12get_max_succEj, .-_ZN21SmallProgressMeasures12get_max_succEj
	.weak	_ZTV21SmallProgressMeasures
	.section	.rodata._ZTV21SmallProgressMeasures,"aG",@progbits,_ZTV21SmallProgressMeasures,comdat
	.align 8
	.type	_ZTV21SmallProgressMeasures, @object
	.size	_ZTV21SmallProgressMeasures, 24
_ZTV21SmallProgressMeasures:
	.long	0
	.long	_ZTI21SmallProgressMeasures
	.long	_ZN21SmallProgressMeasuresD1Ev
	.long	_ZN21SmallProgressMeasuresD0Ev
	.long	_ZN21SmallProgressMeasures5solveEv
	.long	_ZN21SmallProgressMeasures6winnerEj
	.weak	_ZTS21SmallProgressMeasures
	.section	.rodata._ZTS21SmallProgressMeasures,"aG",@progbits,_ZTS21SmallProgressMeasures,comdat
	.type	_ZTS21SmallProgressMeasures, @object
	.size	_ZTS21SmallProgressMeasures, 24
_ZTS21SmallProgressMeasures:
	.string	"21SmallProgressMeasures"
	.weak	_ZTI21SmallProgressMeasures
	.section	.rodata._ZTI21SmallProgressMeasures,"aG",@progbits,_ZTI21SmallProgressMeasures,comdat
	.align 4
	.type	_ZTI21SmallProgressMeasures, @object
	.size	_ZTI21SmallProgressMeasures, 12
_ZTI21SmallProgressMeasures:
	.long	_ZTVN10__cxxabiv120__si_class_type_infoE+8
	.long	_ZTS21SmallProgressMeasures
	.long	_ZTI16ParityGameSolver
	.weak	_ZTI16ParityGameSolver
	.section	.rodata._ZTI16ParityGameSolver,"aG",@progbits,_ZTI16ParityGameSolver,comdat
	.align 4
	.type	_ZTI16ParityGameSolver, @object
	.size	_ZTI16ParityGameSolver, 8
_ZTI16ParityGameSolver:
	.long	_ZTVN10__cxxabiv117__class_type_infoE+8
	.long	_ZTS16ParityGameSolver
	.weak	_ZTS16ParityGameSolver
	.section	.rodata._ZTS16ParityGameSolver,"aG",@progbits,_ZTS16ParityGameSolver,comdat
	.type	_ZTS16ParityGameSolver, @object
	.size	_ZTS16ParityGameSolver, 19
_ZTS16ParityGameSolver:
	.string	"16ParityGameSolver"
	.local	_ZStL8__ioinit
	.comm	_ZStL8__ioinit,1,1
	.section	.rodata
	.align 32
	.type	_ZZN21SmallProgressMeasures12get_ext_succEjbE19__PRETTY_FUNCTION__, @object
	.size	_ZZN21SmallProgressMeasures12get_ext_succEjbE19__PRETTY_FUNCTION__, 55
_ZZN21SmallProgressMeasures12get_ext_succEjbE19__PRETTY_FUNCTION__:
	.string	"verti SmallProgressMeasures::get_ext_succ(verti, bool)"
	.weak	_ZTV16ParityGameSolver
	.section	.rodata._ZTV16ParityGameSolver,"aG",@progbits,_ZTV16ParityGameSolver,comdat
	.align 8
	.type	_ZTV16ParityGameSolver, @object
	.size	_ZTV16ParityGameSolver, 24
_ZTV16ParityGameSolver:
	.long	0
	.long	_ZTI16ParityGameSolver
	.long	_ZN16ParityGameSolverD1Ev
	.long	_ZN16ParityGameSolverD0Ev
	.long	__cxa_pure_virtual
	.long	__cxa_pure_virtual
	.weakref	_ZL20__gthrw_pthread_oncePiPFvvE,pthread_once
	.weakref	_ZL27__gthrw_pthread_getspecificj,pthread_getspecific
	.weakref	_ZL27__gthrw_pthread_setspecificjPKv,pthread_setspecific
	.weakref	_ZL22__gthrw_pthread_createPmPK14pthread_attr_tPFPvS3_ES3_,pthread_create
	.weakref	_ZL22__gthrw_pthread_cancelm,pthread_cancel
	.weakref	_ZL26__gthrw_pthread_mutex_lockP15pthread_mutex_t,pthread_mutex_lock
	.weakref	_ZL29__gthrw_pthread_mutex_trylockP15pthread_mutex_t,pthread_mutex_trylock
	.weakref	_ZL28__gthrw_pthread_mutex_unlockP15pthread_mutex_t,pthread_mutex_unlock
	.weakref	_ZL26__gthrw_pthread_mutex_initP15pthread_mutex_tPK19pthread_mutexattr_t,pthread_mutex_init
	.weakref	_ZL30__gthrw_pthread_cond_broadcastP14pthread_cond_t,pthread_cond_broadcast
	.weakref	_ZL25__gthrw_pthread_cond_waitP14pthread_cond_tP15pthread_mutex_t,pthread_cond_wait
	.weakref	_ZL26__gthrw_pthread_key_createPjPFvPvE,pthread_key_create
	.weakref	_ZL26__gthrw_pthread_key_deletej,pthread_key_delete
	.weakref	_ZL30__gthrw_pthread_mutexattr_initP19pthread_mutexattr_t,pthread_mutexattr_init
	.weakref	_ZL33__gthrw_pthread_mutexattr_settypeP19pthread_mutexattr_ti,pthread_mutexattr_settype
	.weakref	_ZL33__gthrw_pthread_mutexattr_destroyP19pthread_mutexattr_t,pthread_mutexattr_destroy
	.section	.eh_frame,"a",@progbits
.Lframe1:
	.long	.LECIE1-.LSCIE1
.LSCIE1:
	.long	0x0
	.byte	0x1
	.string	"zPL"
	.uleb128 0x1
	.sleb128 -4
	.byte	0x8
	.uleb128 0x6
	.byte	0x0
	.long	__gxx_personality_v0
	.byte	0x0
	.byte	0xc
	.uleb128 0x4
	.uleb128 0x4
	.byte	0x88
	.uleb128 0x1
	.align 4
.LECIE1:
.LSFDE5:
	.long	.LEFDE5-.LASFDE5
.LASFDE5:
	.long	.LASFDE5-.Lframe1
	.long	.LFB1007
	.long	.LFE1007-.LFB1007
	.uleb128 0x4
	.long	0x0
	.byte	0x4
	.long	.LCFI4-.LFB1007
	.byte	0xe
	.uleb128 0x8
	.byte	0x85
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI5-.LCFI4
	.byte	0xd
	.uleb128 0x5
	.byte	0x4
	.long	.LCFI7-.LCFI5
	.byte	0x83
	.uleb128 0x3
	.align 4
.LEFDE5:
.LSFDE9:
	.long	.LEFDE9-.LASFDE9
.LASFDE9:
	.long	.LASFDE9-.Lframe1
	.long	.LFB1010
	.long	.LFE1010-.LFB1010
	.uleb128 0x4
	.long	0x0
	.byte	0x4
	.long	.LCFI10-.LFB1010
	.byte	0xe
	.uleb128 0x8
	.byte	0x85
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI11-.LCFI10
	.byte	0xd
	.uleb128 0x5
	.align 4
.LEFDE9:
.LSFDE21:
	.long	.LEFDE21-.LASFDE21
.LASFDE21:
	.long	.LASFDE21-.Lframe1
	.long	.LFB997
	.long	.LFE997-.LFB997
	.uleb128 0x4
	.long	.LLSDA997
	.byte	0x4
	.long	.LCFI33-.LFB997
	.byte	0xe
	.uleb128 0x8
	.byte	0x85
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI34-.LCFI33
	.byte	0xd
	.uleb128 0x5
	.byte	0x4
	.long	.LCFI38-.LCFI34
	.byte	0x83
	.uleb128 0x5
	.byte	0x86
	.uleb128 0x4
	.byte	0x87
	.uleb128 0x3
	.align 4
.LEFDE21:
.LSFDE23:
	.long	.LEFDE23-.LASFDE23
.LASFDE23:
	.long	.LASFDE23-.Lframe1
	.long	.LFB996
	.long	.LFE996-.LFB996
	.uleb128 0x4
	.long	.LLSDA996
	.byte	0x4
	.long	.LCFI39-.LFB996
	.byte	0xe
	.uleb128 0x8
	.byte	0x85
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI40-.LCFI39
	.byte	0xd
	.uleb128 0x5
	.byte	0x4
	.long	.LCFI44-.LCFI40
	.byte	0x83
	.uleb128 0x5
	.byte	0x86
	.uleb128 0x4
	.byte	0x87
	.uleb128 0x3
	.align 4
.LEFDE23:
	.ident	"GCC: (Gentoo 4.3.2 p1.0) 4.3.2"
	.section	.note.GNU-stack,"",@progbits
