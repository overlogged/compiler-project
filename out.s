	.section	__TEXT,__text,regular,pure_instructions
	.macosx_version_min 10, 14
	.globl	_main                   ## -- Begin function main
	.p2align	4, 0x90
_main:                                  ## @main
	.cfi_startproc
## %bb.0:                               ## %entry
	pushq	%rax
	.cfi_def_cfa_offset 16
	movl	$0, 4(%rsp)
	movq	_x@GOTPCREL(%rip), %rax
	movl	$0, (%rax)
	movl	$21, %edi
	callq	_fun
	movq	_y@GOTPCREL(%rip), %rcx
	movb	%al, (%rcx)
	callq	_.main
	movl	%eax, 4(%rsp)
	movl	4(%rsp), %eax
	popq	%rcx
	retq
	.cfi_endproc
                                        ## -- End function
	.globl	_test                   ## -- Begin function test
	.p2align	4, 0x90
_test:                                  ## @test
	.cfi_startproc
## %bb.0:                               ## %entry
	movb	$0, -6(%rsp)
	movb	%dil, -5(%rsp)
	movl	%esi, -4(%rsp)
	movb	-6(%rsp), %al
	retq
	.cfi_endproc
                                        ## -- End function
	.globl	_fun                    ## -- Begin function fun
	.p2align	4, 0x90
_fun:                                   ## @fun
	.cfi_startproc
## %bb.0:                               ## %entry
	movb	$0, -2(%rsp)
	movb	%dil, -1(%rsp)
	testb	%dil, %dil
	movb	-2(%rsp), %al
	retq
	.cfi_endproc
                                        ## -- End function
	.globl	_.main                  ## -- Begin function .main
	.p2align	4, 0x90
_.main:                                 ## @.main
	.cfi_startproc
## %bb.0:                               ## %entry
	pushq	%rax
	.cfi_def_cfa_offset 16
	movl	$0, 4(%rsp)
	movb	$1, 2(%rsp)
	movzbl	2(%rsp), %edi
	callq	_fun
	movb	%al, 3(%rsp)
	movl	4(%rsp), %eax
	popq	%rcx
	retq
	.cfi_endproc
                                        ## -- End function
	.comm	_x,4,2                  ## @x
	.comm	_y,1,2                  ## @y

.subsections_via_symbols
