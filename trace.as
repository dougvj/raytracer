	.file	"trace.c"
	.text
	.globl	noHit
	.type	noHit, @function
noHit:
.LFB9:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	%rax, %rsi
	movl	$0, %eax
	movl	$8, %edx
	movq	%rsi, %rdi
	movq	%rdx, %rcx
	rep stosq
	movq	-8(%rbp), %rax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE9:
	.size	noHit, .-noHit
	.globl	Hit
	.type	Hit, @function
Hit:
.LFB10:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	%xmm0, %rax
	movq	%xmm1, %rcx
	movq	%rcx, %rdx
	movq	%rax, -32(%rbp)
	movq	%rdx, -24(%rbp)
	movq	%xmm2, %rax
	movq	%xmm3, %rcx
	movq	%rcx, %rdx
	movq	%rax, -48(%rbp)
	movq	%rdx, -40(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-8(%rbp), %rax
	movl	$1, (%rax)
	movq	-8(%rbp), %rcx
	movq	-48(%rbp), %rax
	movq	-40(%rbp), %rdx
	movq	%rax, 16(%rcx)
	movq	%rdx, 24(%rcx)
	movq	-8(%rbp), %rcx
	movq	-32(%rbp), %rax
	movq	-24(%rbp), %rdx
	movq	%rax, 32(%rcx)
	movq	%rdx, 40(%rcx)
	movq	-8(%rbp), %rax
	movq	-16(%rbp), %rdx
	movq	%rdx, 48(%rax)
	movq	-8(%rbp), %rax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE10:
	.size	Hit, .-Hit
	.globl	intersectSphere
	.type	intersectSphere, @function
intersectSphere:
.LFB11:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$160, %rsp
	movq	%rdi, -136(%rbp)
	movq	%rsi, -144(%rbp)
	movq	%rdx, -152(%rbp)
	movq	-144(%rbp), %rax
	movaps	(%rax), %xmm0
	movq	-152(%rbp), %rax
	movaps	(%rax), %xmm1
	subps	%xmm1, %xmm0
	movaps	%xmm0, -80(%rbp)
	movq	-80(%rbp), %rsi
	movq	-72(%rbp), %rcx
	movq	-152(%rbp), %rax
	movq	16(%rax), %rdx
	movq	24(%rax), %rax
	movq	%rsi, -160(%rbp)
	movq	-160(%rbp), %xmm2
	movq	%rcx, -160(%rbp)
	movq	-160(%rbp), %xmm3
	movq	%rdx, -160(%rbp)
	movq	-160(%rbp), %xmm0
	movq	%rax, -160(%rbp)
	movq	-160(%rbp), %xmm1
	call	dot
	movaps	%xmm0, %xmm1
	pxor	%xmm0, %xmm0
	ucomiss	%xmm1, %xmm0
	jb	.L14
	movq	-136(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	noHit
	jmp	.L5
.L14:
	movq	-152(%rbp), %rax
	movq	16(%rax), %rsi
	movq	24(%rax), %rcx
	movq	-152(%rbp), %rax
	movq	16(%rax), %rdx
	movq	24(%rax), %rax
	movq	%rsi, -160(%rbp)
	movq	-160(%rbp), %xmm2
	movq	%rcx, -160(%rbp)
	movq	-160(%rbp), %xmm3
	movq	%rdx, -160(%rbp)
	movq	-160(%rbp), %xmm0
	movq	%rax, -160(%rbp)
	movq	-160(%rbp), %xmm1
	call	dot
	movd	%xmm0, %eax
	movl	%eax, -4(%rbp)
	movq	-152(%rbp), %rax
	movaps	(%rax), %xmm0
	movq	-144(%rbp), %rax
	movaps	(%rax), %xmm1
	subps	%xmm1, %xmm0
	movaps	%xmm0, -96(%rbp)
	movaps	.LC1(%rip), %xmm0
	movaps	%xmm0, -48(%rbp)
	movaps	-48(%rbp), %xmm1
	movq	-152(%rbp), %rax
	movaps	16(%rax), %xmm0
	mulps	%xmm1, %xmm0
	movaps	%xmm0, -64(%rbp)
	movq	-96(%rbp), %rsi
	movq	-88(%rbp), %rcx
	movq	-64(%rbp), %rdx
	movq	-56(%rbp), %rax
	movq	%rsi, -160(%rbp)
	movq	-160(%rbp), %xmm2
	movq	%rcx, -160(%rbp)
	movq	-160(%rbp), %xmm3
	movq	%rdx, -160(%rbp)
	movq	-160(%rbp), %xmm0
	movq	%rax, -160(%rbp)
	movq	-160(%rbp), %xmm1
	call	dot
	movd	%xmm0, %eax
	movl	%eax, -8(%rbp)
	movq	-96(%rbp), %rsi
	movq	-88(%rbp), %rcx
	movq	-96(%rbp), %rdx
	movq	-88(%rbp), %rax
	movq	%rsi, -160(%rbp)
	movq	-160(%rbp), %xmm2
	movq	%rcx, -160(%rbp)
	movq	-160(%rbp), %xmm3
	movq	%rdx, -160(%rbp)
	movq	-160(%rbp), %xmm0
	movq	%rax, -160(%rbp)
	movq	-160(%rbp), %xmm1
	call	dot
	movaps	%xmm0, %xmm2
	movq	-144(%rbp), %rax
	movss	16(%rax), %xmm1
	movq	-144(%rbp), %rax
	movss	16(%rax), %xmm0
	mulss	%xmm1, %xmm0
	subss	%xmm0, %xmm2
	movaps	%xmm2, %xmm0
	movss	%xmm0, -12(%rbp)
	movss	-8(%rbp), %xmm0
	mulss	-8(%rbp), %xmm0
	movss	-4(%rbp), %xmm2
	movss	.LC2(%rip), %xmm1
	mulss	%xmm2, %xmm1
	mulss	-12(%rbp), %xmm1
	subss	%xmm1, %xmm0
	movss	%xmm0, -16(%rbp)
	movss	-16(%rbp), %xmm0
	pxor	%xmm1, %xmm1
	ucomiss	%xmm1, %xmm0
	jb	.L15
	movss	-8(%rbp), %xmm1
	movss	.LC3(%rip), %xmm0
	xorps	%xmm1, %xmm0
	cvtss2sd	%xmm0, %xmm5
	movsd	%xmm5, -160(%rbp)
	cvtss2sd	-16(%rbp), %xmm0
	call	sqrt
	movsd	-160(%rbp), %xmm5
	subsd	%xmm0, %xmm5
	movapd	%xmm5, %xmm1
	movss	-4(%rbp), %xmm0
	addss	%xmm0, %xmm0
	cvtss2sd	%xmm0, %xmm0
	divsd	%xmm0, %xmm1
	movapd	%xmm1, %xmm0
	cvtsd2ss	%xmm0, %xmm6
	movss	%xmm6, -20(%rbp)
	movq	-152(%rbp), %rax
	movss	(%rax), %xmm1
	movq	-152(%rbp), %rax
	movss	16(%rax), %xmm0
	mulss	-20(%rbp), %xmm0
	addss	%xmm0, %xmm1
	movq	-152(%rbp), %rax
	movss	4(%rax), %xmm2
	movq	-152(%rbp), %rax
	movss	20(%rax), %xmm0
	mulss	-20(%rbp), %xmm0
	addss	%xmm2, %xmm0
	movq	-152(%rbp), %rax
	movss	8(%rax), %xmm3
	movq	-152(%rbp), %rax
	movss	24(%rax), %xmm2
	mulss	-20(%rbp), %xmm2
	addss	%xmm3, %xmm2
	pxor	%xmm3, %xmm3
	unpcklps	%xmm3, %xmm2
	movaps	%xmm1, %xmm4
	unpcklps	%xmm0, %xmm4
	movaps	%xmm4, %xmm0
	movlhps	%xmm2, %xmm0
	movaps	%xmm0, -112(%rbp)
	movq	-144(%rbp), %rax
	movaps	(%rax), %xmm0
	movaps	-112(%rbp), %xmm1
	subps	%xmm1, %xmm0
	movaps	%xmm0, -128(%rbp)
	movq	-144(%rbp), %rax
	leaq	32(%rax), %rsi
	movq	-136(%rbp), %rax
	movq	-128(%rbp), %r8
	movq	-120(%rbp), %rdi
	movq	-112(%rbp), %rcx
	movq	-104(%rbp), %rdx
	movq	%r8, -160(%rbp)
	movq	-160(%rbp), %xmm2
	movq	%rdi, -160(%rbp)
	movq	-160(%rbp), %xmm3
	movq	%rcx, -160(%rbp)
	movq	-160(%rbp), %xmm0
	movq	%rdx, -160(%rbp)
	movq	-160(%rbp), %xmm1
	movq	%rax, %rdi
	call	Hit
	jmp	.L5
.L15:
	movq	-136(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	noHit
.L5:
	movq	-136(%rbp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE11:
	.size	intersectSphere, .-intersectSphere
	.globl	intersectPlane
	.type	intersectPlane, @function
intersectPlane:
.LFB12:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$112, %rsp
	movq	%rdi, -88(%rbp)
	movq	%rsi, -96(%rbp)
	movq	%rdx, -104(%rbp)
	movq	-96(%rbp), %rax
	movaps	(%rax), %xmm0
	movq	-104(%rbp), %rax
	movaps	(%rax), %xmm1
	subps	%xmm1, %xmm0
	movaps	%xmm0, -64(%rbp)
	movq	-64(%rbp), %rsi
	movq	-56(%rbp), %rcx
	movq	-96(%rbp), %rax
	movq	16(%rax), %rdx
	movq	24(%rax), %rax
	movq	%rsi, -112(%rbp)
	movq	-112(%rbp), %xmm2
	movq	%rcx, -112(%rbp)
	movq	-112(%rbp), %xmm3
	movq	%rdx, -112(%rbp)
	movq	-112(%rbp), %xmm0
	movq	%rax, -112(%rbp)
	movq	-112(%rbp), %xmm1
	call	dot
	movd	%xmm0, %eax
	movl	%eax, -4(%rbp)
	movq	-104(%rbp), %rax
	movaps	16(%rax), %xmm0
	movaps	.LC4(%rip), %xmm1
	xorps	%xmm1, %xmm0
	movaps	%xmm0, -48(%rbp)
	movq	-48(%rbp), %rsi
	movq	-40(%rbp), %rcx
	movq	-96(%rbp), %rax
	movq	16(%rax), %rdx
	movq	24(%rax), %rax
	movq	%rsi, -112(%rbp)
	movq	-112(%rbp), %xmm2
	movq	%rcx, -112(%rbp)
	movq	-112(%rbp), %xmm3
	movq	%rdx, -112(%rbp)
	movq	-112(%rbp), %xmm0
	movq	%rax, -112(%rbp)
	movq	-112(%rbp), %xmm1
	call	dot
	movd	%xmm0, %eax
	movl	%eax, -8(%rbp)
	pxor	%xmm0, %xmm0
	ucomiss	-8(%rbp), %xmm0
	jp	.L17
	pxor	%xmm0, %xmm0
	ucomiss	-8(%rbp), %xmm0
	jne	.L17
	movq	-88(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	noHit
	jmp	.L16
.L17:
	movss	-4(%rbp), %xmm0
	divss	-8(%rbp), %xmm0
	movss	%xmm0, -12(%rbp)
	movss	.LC5(%rip), %xmm0
	ucomiss	-12(%rbp), %xmm0
	jb	.L25
	movq	-88(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	noHit
	jmp	.L16
.L25:
	movq	-104(%rbp), %rax
	movaps	(%rax), %xmm1
	movss	-12(%rbp), %xmm0
	shufps	$0, %xmm0, %xmm0
	movaps	%xmm0, -32(%rbp)
	movaps	-32(%rbp), %xmm2
	movq	-104(%rbp), %rax
	movaps	16(%rax), %xmm0
	mulps	%xmm2, %xmm0
	addps	%xmm1, %xmm0
	movaps	%xmm0, -80(%rbp)
	movq	-96(%rbp), %rax
	leaq	32(%rax), %rsi
	movq	-88(%rbp), %rdx
	movq	-96(%rbp), %rax
	movq	16(%rax), %r8
	movq	24(%rax), %rdi
	movq	-80(%rbp), %rcx
	movq	-72(%rbp), %rax
	movq	%r8, -112(%rbp)
	movq	-112(%rbp), %xmm2
	movq	%rdi, -112(%rbp)
	movq	-112(%rbp), %xmm3
	movq	%rcx, -112(%rbp)
	movq	-112(%rbp), %xmm0
	movq	%rax, -112(%rbp)
	movq	-112(%rbp), %xmm1
	movq	%rdx, %rdi
	call	Hit
.L16:
	movq	-88(%rbp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE12:
	.size	intersectPlane, .-intersectPlane
	.globl	intersectTriangle
	.type	intersectTriangle, @function
intersectTriangle:
.LFB13:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$24, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	%rdx, -24(%rbp)
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	noHit
	movq	-8(%rbp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE13:
	.size	intersectTriangle, .-intersectTriangle
	.globl	intersectEntity
	.type	intersectEntity, @function
intersectEntity:
.LFB14:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	%rdx, -24(%rbp)
	movq	-16(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$2, %eax
	je	.L30
	cmpl	$3, %eax
	je	.L31
	cmpl	$1, %eax
	je	.L32
	jmp	.L34
.L31:
	movq	-16(%rbp), %rax
	movq	8(%rax), %rcx
	movq	-8(%rbp), %rax
	movq	-24(%rbp), %rdx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	intersectTriangle
	jmp	.L28
.L30:
	movq	-16(%rbp), %rax
	movq	8(%rax), %rcx
	movq	-8(%rbp), %rax
	movq	-24(%rbp), %rdx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	intersectSphere
	jmp	.L28
.L32:
	movq	-16(%rbp), %rax
	movq	8(%rax), %rcx
	movq	-8(%rbp), %rax
	movq	-24(%rbp), %rdx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	intersectPlane
	jmp	.L28
.L34:
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	noHit
.L28:
	movq	-8(%rbp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE14:
	.size	intersectEntity, .-intersectEntity
	.globl	findDiffuse
	.type	findDiffuse, @function
findDiffuse:
.LFB15:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	.cfi_offset 3, -24
	movq	%rdi, -24(%rbp)
	movq	%xmm0, %rax
	movq	%xmm1, %rcx
	movq	%rcx, %rdx
	movq	%rax, -48(%rbp)
	movq	%rdx, -40(%rbp)
	movq	%xmm2, %rax
	movq	%xmm3, %rcx
	movq	%rcx, %rdx
	movq	%rax, -64(%rbp)
	movq	%rdx, -56(%rbp)
	movl	$0, %eax
	movl	$0, %edx
	movq	%rax, %rcx
	movq	%rdx, %rbx
	movq	%rdx, %rax
	movq	%rcx, -32(%rbp)
	movq	-32(%rbp), %xmm0
	movq	%rax, -32(%rbp)
	movq	-32(%rbp), %xmm1
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE15:
	.size	findDiffuse, .-findDiffuse
	.globl	getBackground
	.type	getBackground, @function
getBackground:
.LFB16:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	.cfi_offset 3, -24
	movl	$0, %eax
	movl	$0, %edx
	movq	%rax, %rcx
	movq	%rdx, %rbx
	movq	%rdx, %rax
	movq	%rcx, -24(%rbp)
	movq	-24(%rbp), %xmm0
	movq	%rax, -24(%rbp)
	movq	-24(%rbp), %xmm1
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE16:
	.size	getBackground, .-getBackground
	.globl	_traceRay
	.type	_traceRay, @function
_traceRay:
.LFB17:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$280, %rsp
	.cfi_offset 3, -24
	movq	%rdi, -264(%rbp)
	movl	%esi, -268(%rbp)
	cmpl	$20, -268(%rbp)
	jle	.L40
	pushq	40(%rbp)
	pushq	32(%rbp)
	pushq	24(%rbp)
	pushq	16(%rbp)
	call	getBackground
	addq	$32, %rsp
	movq	%xmm0, %rax
	movq	%xmm1, %rcx
	movq	%rcx, %rdx
	jmp	.L49
.L40:
	movq	-264(%rbp), %rax
	movq	16(%rax), %rax
	movq	%rax, %rdi
	call	llInitIterator
	movq	%rax, -40(%rbp)
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	call	llGetNext
	movq	%rax, -24(%rbp)
	leaq	-160(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	noHit
	jmp	.L42
.L47:
	leaq	-256(%rbp), %rax
	movq	-24(%rbp), %rcx
	leaq	16(%rbp), %rdx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	intersectEntity
	movl	-256(%rbp), %eax
	testl	%eax, %eax
	je	.L43
	movaps	-224(%rbp), %xmm0
	movaps	16(%rbp), %xmm1
	subps	%xmm1, %xmm0
	movaps	%xmm0, -96(%rbp)
	movq	-96(%rbp), %rdx
	movq	-88(%rbp), %rax
	movq	%rdx, -288(%rbp)
	movq	-288(%rbp), %xmm0
	movq	%rax, -288(%rbp)
	movq	-288(%rbp), %xmm1
	call	length
	movd	%xmm0, %eax
	movl	%eax, -44(%rbp)
	movl	-160(%rbp), %eax
	testl	%eax, %eax
	je	.L44
	movss	-28(%rbp), %xmm0
	ucomiss	-44(%rbp), %xmm0
	jbe	.L43
	movq	-256(%rbp), %rax
	movq	%rax, -160(%rbp)
	movq	-248(%rbp), %rax
	movq	%rax, -152(%rbp)
	movq	-240(%rbp), %rax
	movq	%rax, -144(%rbp)
	movq	-232(%rbp), %rax
	movq	%rax, -136(%rbp)
	movq	-224(%rbp), %rax
	movq	%rax, -128(%rbp)
	movq	-216(%rbp), %rax
	movq	%rax, -120(%rbp)
	movq	-208(%rbp), %rax
	movq	%rax, -112(%rbp)
	movq	-200(%rbp), %rax
	movq	%rax, -104(%rbp)
	movss	-44(%rbp), %xmm0
	movss	%xmm0, -28(%rbp)
	jmp	.L43
.L44:
	movq	-256(%rbp), %rax
	movq	%rax, -160(%rbp)
	movq	-248(%rbp), %rax
	movq	%rax, -152(%rbp)
	movq	-240(%rbp), %rax
	movq	%rax, -144(%rbp)
	movq	-232(%rbp), %rax
	movq	%rax, -136(%rbp)
	movq	-224(%rbp), %rax
	movq	%rax, -128(%rbp)
	movq	-216(%rbp), %rax
	movq	%rax, -120(%rbp)
	movq	-208(%rbp), %rax
	movq	%rax, -112(%rbp)
	movq	-200(%rbp), %rax
	movq	%rax, -104(%rbp)
	movss	-44(%rbp), %xmm0
	movss	%xmm0, -28(%rbp)
.L43:
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	call	llGetNext
	movq	%rax, -24(%rbp)
.L42:
	cmpq	$0, -24(%rbp)
	jne	.L47
	movl	-160(%rbp), %eax
	testl	%eax, %eax
	je	.L48
	movl	-268(%rbp), %eax
	leal	1(%rax), %ebx
	movq	-128(%rbp), %rax
	movq	-120(%rbp), %rdx
	movq	%rax, -80(%rbp)
	movq	%rdx, -72(%rbp)
	movq	-144(%rbp), %rsi
	movq	-136(%rbp), %rcx
	movq	32(%rbp), %rdx
	movq	40(%rbp), %rax
	movq	%rsi, -288(%rbp)
	movq	-288(%rbp), %xmm2
	movq	%rcx, -288(%rbp)
	movq	-288(%rbp), %xmm3
	movq	%rdx, -288(%rbp)
	movq	-288(%rbp), %xmm0
	movq	%rax, -288(%rbp)
	movq	-288(%rbp), %xmm1
	call	reflect
	movq	%xmm0, %rax
	movq	%xmm1, %rdx
	movq	%rax, -64(%rbp)
	movq	%rdx, -56(%rbp)
	movq	-264(%rbp), %rax
	pushq	-56(%rbp)
	pushq	-64(%rbp)
	pushq	-72(%rbp)
	pushq	-80(%rbp)
	movl	%ebx, %esi
	movq	%rax, %rdi
	call	_traceRay
	addq	$32, %rsp
	movq	%xmm0, %rax
	movq	%xmm1, %rdx
	movq	%rax, -176(%rbp)
	movq	%rdx, -168(%rbp)
	movq	-144(%rbp), %rdi
	movq	-136(%rbp), %rsi
	movq	-128(%rbp), %rcx
	movq	-120(%rbp), %rdx
	movq	-264(%rbp), %rax
	movq	%rdi, -288(%rbp)
	movq	-288(%rbp), %xmm2
	movq	%rsi, -288(%rbp)
	movq	-288(%rbp), %xmm3
	movq	%rcx, -288(%rbp)
	movq	-288(%rbp), %xmm0
	movq	%rdx, -288(%rbp)
	movq	-288(%rbp), %xmm1
	movq	%rax, %rdi
	call	findDiffuse
	movq	%xmm0, %rax
	movq	%xmm1, %rdx
	movq	%rax, -192(%rbp)
	movq	%rdx, -184(%rbp)
	movq	-112(%rbp), %rax
	movaps	(%rax), %xmm1
	movaps	-176(%rbp), %xmm0
	mulps	%xmm0, %xmm1
	movq	-112(%rbp), %rax
	movaps	16(%rax), %xmm2
	movaps	-192(%rbp), %xmm0
	mulps	%xmm2, %xmm0
	addps	%xmm0, %xmm1
	movq	-112(%rbp), %rax
	movaps	32(%rax), %xmm0
	addps	%xmm1, %xmm0
	movaps	%xmm0, -288(%rbp)
	movq	-288(%rbp), %rax
	movq	-280(%rbp), %rdx
	jmp	.L49
.L48:
	pushq	40(%rbp)
	pushq	32(%rbp)
	pushq	24(%rbp)
	pushq	16(%rbp)
	call	getBackground
	addq	$32, %rsp
	movq	%xmm0, %rax
	movq	%xmm1, %rcx
	movq	%rcx, %rdx
.L49:
	movq	%rax, %rcx
	movq	%rdx, %rbx
	movq	%rdx, %rax
	movq	%rcx, -288(%rbp)
	movq	-288(%rbp), %xmm0
	movq	%rax, -288(%rbp)
	movq	-288(%rbp), %xmm1
	movq	-8(%rbp), %rbx
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE17:
	.size	_traceRay, .-_traceRay
	.globl	convertFloatToColor
	.type	convertFloatToColor, @function
convertFloatToColor:
.LFB18:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%xmm0, %rax
	movq	%xmm1, %rcx
	movq	%rcx, %rdx
	movq	%rax, -48(%rbp)
	movq	%rdx, -40(%rbp)
	movss	-48(%rbp), %xmm0
	movss	.LC5(%rip), %xmm1
	ucomiss	%xmm1, %xmm0
	jbe	.L52
	movss	.LC5(%rip), %xmm0
	movss	%xmm0, -48(%rbp)
.L52:
	movss	-44(%rbp), %xmm0
	movss	.LC5(%rip), %xmm1
	ucomiss	%xmm1, %xmm0
	jbe	.L54
	movss	.LC5(%rip), %xmm0
	movss	%xmm0, -44(%rbp)
.L54:
	movss	-40(%rbp), %xmm0
	movss	.LC5(%rip), %xmm1
	ucomiss	%xmm1, %xmm0
	jbe	.L56
	movss	.LC5(%rip), %xmm0
	movss	%xmm0, -40(%rbp)
.L56:
	movss	-48(%rbp), %xmm1
	movss	.LC6(%rip), %xmm0
	mulss	%xmm1, %xmm0
	cvttss2si	%xmm0, %eax
	movb	%al, -30(%rbp)
	movss	-44(%rbp), %xmm1
	movss	.LC6(%rip), %xmm0
	mulss	%xmm1, %xmm0
	cvttss2si	%xmm0, %eax
	movb	%al, -31(%rbp)
	movss	-40(%rbp), %xmm1
	movss	.LC6(%rip), %xmm0
	mulss	%xmm1, %xmm0
	cvttss2si	%xmm0, %eax
	movb	%al, -32(%rbp)
	movzwl	-32(%rbp), %eax
	movw	%ax, -16(%rbp)
	movzbl	-30(%rbp), %eax
	movb	%al, -14(%rbp)
	movl	$0, %eax
	movzbl	-16(%rbp), %edx
	movzbl	%dl, %edx
	movb	$0, %al
	orq	%rdx, %rax
	movzbl	-15(%rbp), %edx
	movb	%dl, %ah
	movzbl	-14(%rbp), %edx
	movzbl	%dl, %edx
	salq	$16, %rdx
	andq	$-16711681, %rax
	orq	%rdx, %rax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE18:
	.size	convertFloatToColor, .-convertFloatToColor
	.globl	_renderPixel
	.type	_renderPixel, @function
_renderPixel:
.LFB19:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$104, %rsp
	.cfi_offset 3, -24
	movq	%rdi, -88(%rbp)
	movl	%esi, -92(%rbp)
	movl	%edx, -96(%rbp)
	pxor	%xmm0, %xmm0
	cvtsi2ss	-92(%rbp), %xmm0
	movq	-88(%rbp), %rax
	movl	4(%rax), %eax
	pxor	%xmm1, %xmm1
	cvtsi2ss	%eax, %xmm1
	divss	%xmm1, %xmm0
	addss	%xmm0, %xmm0
	movss	.LC5(%rip), %xmm1
	subss	%xmm1, %xmm0
	movss	%xmm0, -20(%rbp)
	pxor	%xmm0, %xmm0
	cvtsi2ss	-96(%rbp), %xmm0
	movq	-88(%rbp), %rax
	movl	8(%rax), %eax
	pxor	%xmm1, %xmm1
	cvtsi2ss	%eax, %xmm1
	divss	%xmm1, %xmm0
	addss	%xmm0, %xmm0
	movss	.LC5(%rip), %xmm1
	subss	%xmm1, %xmm0
	movaps	%xmm0, %xmm1
	movq	-88(%rbp), %rax
	movl	8(%rax), %eax
	pxor	%xmm0, %xmm0
	cvtsi2ss	%eax, %xmm0
	movq	-88(%rbp), %rax
	movl	4(%rax), %eax
	pxor	%xmm2, %xmm2
	cvtsi2ss	%eax, %xmm2
	divss	%xmm2, %xmm0
	mulss	%xmm1, %xmm0
	movss	%xmm0, -24(%rbp)
	movq	-88(%rbp), %rax
	movss	12(%rax), %xmm0
	movss	.LC7(%rip), %xmm1
	divss	%xmm1, %xmm0
	cvtss2sd	%xmm0, %xmm0
	call	tan
	movapd	%xmm0, %xmm1
	movsd	.LC8(%rip), %xmm0
	divsd	%xmm1, %xmm0
	cvtsd2ss	%xmm0, %xmm4
	movss	%xmm4, -28(%rbp)
	pxor	%xmm0, %xmm0
	movss	-28(%rbp), %xmm1
	movaps	%xmm1, %xmm2
	unpcklps	%xmm0, %xmm2
	movss	-24(%rbp), %xmm0
	movss	-20(%rbp), %xmm1
	movaps	%xmm1, %xmm3
	unpcklps	%xmm0, %xmm3
	movaps	%xmm3, %xmm0
	movlhps	%xmm2, %xmm0
	movaps	%xmm0, -48(%rbp)
	pxor	%xmm0, %xmm0
	movaps	%xmm0, -80(%rbp)
	movq	-48(%rbp), %rax
	movq	-40(%rbp), %rdx
	movq	%rax, -64(%rbp)
	movq	%rdx, -56(%rbp)
	movq	-88(%rbp), %rax
	pushq	-56(%rbp)
	pushq	-64(%rbp)
	pushq	-72(%rbp)
	pushq	-80(%rbp)
	movl	$0, %esi
	movq	%rax, %rdi
	call	_traceRay
	addq	$32, %rsp
	movq	%xmm0, %rax
	movq	%xmm1, %rcx
	movq	%rcx, %rdx
	movq	%rax, %rcx
	movq	%rdx, %rbx
	movq	%rdx, %rax
	movq	%rcx, -104(%rbp)
	movq	-104(%rbp), %xmm0
	movq	%rax, -104(%rbp)
	movq	-104(%rbp), %xmm1
	call	convertFloatToColor
	cltq
	movb	%al, -32(%rbp)
	movzbl	%ah, %edx
	movb	%dl, -31(%rbp)
	shrq	$16, %rax
	andb	$255, %ah
	movb	%al, -30(%rbp)
	movl	$0, %eax
	movzbl	-32(%rbp), %edx
	movzbl	%dl, %edx
	movb	$0, %al
	orq	%rdx, %rax
	movzbl	-31(%rbp), %edx
	movb	%dl, %ah
	movzbl	-30(%rbp), %edx
	movzbl	%dl, %edx
	salq	$16, %rdx
	andq	$-16711681, %rax
	orq	%rdx, %rax
	movq	-8(%rbp), %rbx
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE19:
	.size	_renderPixel, .-_renderPixel
	.globl	_startRenderThread
	.type	_startRenderThread, @function
_startRenderThread:
.LFB20:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$72, %rsp
	.cfi_offset 3, -24
	movq	%rdi, -72(%rbp)
	movq	-72(%rbp), %rax
	movq	(%rax), %rax
	movl	4(%rax), %edx
	movq	-72(%rbp), %rax
	movq	(%rax), %rax
	movl	8(%rax), %eax
	imull	%edx, %eax
	cltq
	movq	%rax, -32(%rbp)
	movq	-72(%rbp), %rax
	movq	(%rax), %rax
	movl	(%rax), %eax
	movslq	%eax, %rsi
	movq	-32(%rbp), %rax
	cqto
	idivq	%rsi
	movq	%rax, -40(%rbp)
	movq	-72(%rbp), %rax
	movl	8(%rax), %eax
	cltq
	imulq	-40(%rbp), %rax
	movq	%rax, -48(%rbp)
	movq	-72(%rbp), %rax
	movl	8(%rax), %eax
	addl	$1, %eax
	cltq
	imulq	-40(%rbp), %rax
	movq	%rax, -56(%rbp)
	movq	-48(%rbp), %rax
	movq	%rax, -24(%rbp)
	jmp	.L65
.L66:
	movq	-72(%rbp), %rax
	movq	(%rax), %rax
	movl	4(%rax), %eax
	movslq	%eax, %rcx
	movq	-24(%rbp), %rax
	cqto
	idivq	%rcx
	movq	%rdx, %rax
	movl	%eax, -60(%rbp)
	movq	-72(%rbp), %rax
	movq	(%rax), %rax
	movl	4(%rax), %eax
	movslq	%eax, %rdi
	movq	-24(%rbp), %rax
	cqto
	idivq	%rdi
	movl	%eax, -64(%rbp)
	movq	-72(%rbp), %rax
	movq	(%rax), %rax
	movq	24(%rax), %rcx
	movq	-24(%rbp), %rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	leaq	(%rcx,%rax), %rbx
	movq	-72(%rbp), %rax
	movq	(%rax), %rax
	movl	-64(%rbp), %edx
	movl	-60(%rbp), %ecx
	movl	%ecx, %esi
	movq	%rax, %rdi
	call	_renderPixel
	cltq
	movb	%al, (%rbx)
	movzbl	%ah, %edx
	movb	%dl, 1(%rbx)
	shrq	$16, %rax
	andb	$255, %ah
	movb	%al, 2(%rbx)
	addq	$1, -24(%rbp)
.L65:
	movq	-24(%rbp), %rax
	cmpq	-56(%rbp), %rax
	jl	.L66
	nop
	addq	$72, %rsp
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE20:
	.size	_startRenderThread, .-_startRenderThread
	.globl	createRenderContext
	.type	createRenderContext, @function
createRenderContext:
.LFB21:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movl	$32, %edi
	call	malloc
	movq	%rax, -8(%rbp)
	movl	$0, %eax
	call	llCreate
	movq	%rax, %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, 16(%rax)
	movq	-8(%rbp), %rax
	movss	.LC9(%rip), %xmm0
	movss	%xmm0, 12(%rax)
	movq	-8(%rbp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE21:
	.size	createRenderContext, .-createRenderContext
	.globl	createSphere
	.type	createSphere, @function
createSphere:
.LFB22:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movq	%xmm0, %rax
	movq	%xmm1, %rcx
	movq	%rcx, %rdx
	movq	%rax, -32(%rbp)
	movq	%rdx, -24(%rbp)
	movss	%xmm2, -36(%rbp)
	movl	$80, %edi
	call	malloc
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rcx
	movq	-32(%rbp), %rax
	movq	-24(%rbp), %rdx
	movq	%rax, (%rcx)
	movq	%rdx, 8(%rcx)
	movq	-8(%rbp), %rax
	movss	-36(%rbp), %xmm0
	movss	%xmm0, 16(%rax)
	movl	$16, %edi
	call	malloc
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rax
	movq	-8(%rbp), %rdx
	movq	%rdx, 8(%rax)
	movq	-16(%rbp), %rax
	movl	$2, (%rax)
	movq	-16(%rbp), %rax
	movq	8(%rax), %rax
	movq	16(%rbp), %rdx
	movq	%rdx, 32(%rax)
	movq	24(%rbp), %rdx
	movq	%rdx, 40(%rax)
	movq	32(%rbp), %rdx
	movq	%rdx, 48(%rax)
	movq	40(%rbp), %rdx
	movq	%rdx, 56(%rax)
	movq	48(%rbp), %rdx
	movq	%rdx, 64(%rax)
	movq	56(%rbp), %rdx
	movq	%rdx, 72(%rax)
	movq	-16(%rbp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE22:
	.size	createSphere, .-createSphere
	.globl	createPlane
	.type	createPlane, @function
createPlane:
.LFB23:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movq	%xmm0, %rax
	movq	%xmm1, %rcx
	movq	%rcx, %rdx
	movq	%rax, -32(%rbp)
	movq	%rdx, -24(%rbp)
	movq	%xmm2, %rax
	movq	%xmm3, %rcx
	movq	%rcx, %rdx
	movq	%rax, -48(%rbp)
	movq	%rdx, -40(%rbp)
	movl	$80, %edi
	call	malloc
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rcx
	movq	-32(%rbp), %rax
	movq	-24(%rbp), %rdx
	movq	%rax, (%rcx)
	movq	%rdx, 8(%rcx)
	movq	-8(%rbp), %rcx
	movq	-48(%rbp), %rax
	movq	-40(%rbp), %rdx
	movq	%rax, 16(%rcx)
	movq	%rdx, 24(%rcx)
	movl	$16, %edi
	call	malloc
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rax
	movq	-8(%rbp), %rdx
	movq	%rdx, 8(%rax)
	movq	-16(%rbp), %rax
	movl	$1, (%rax)
	movq	-16(%rbp), %rax
	movq	8(%rax), %rax
	movq	16(%rbp), %rdx
	movq	%rdx, 32(%rax)
	movq	24(%rbp), %rdx
	movq	%rdx, 40(%rax)
	movq	32(%rbp), %rdx
	movq	%rdx, 48(%rax)
	movq	40(%rbp), %rdx
	movq	%rdx, 56(%rax)
	movq	48(%rbp), %rdx
	movq	%rdx, 64(%rax)
	movq	56(%rbp), %rdx
	movq	%rdx, 72(%rax)
	movq	-16(%rbp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE23:
	.size	createPlane, .-createPlane
	.globl	addEntity
	.type	addEntity, @function
addEntity:
.LFB24:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-8(%rbp), %rax
	movq	16(%rax), %rax
	movq	-16(%rbp), %rdx
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	llPushBack
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE24:
	.size	addEntity, .-addEntity
	.section	.rodata
.LC10:
	.string	"render.bmp"
	.text
	.globl	renderScene
	.type	renderScene, @function
renderScene:
.LFB25:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$88, %rsp
	.cfi_offset 15, -24
	.cfi_offset 14, -32
	.cfi_offset 13, -40
	.cfi_offset 12, -48
	.cfi_offset 3, -56
	movq	%rdi, -104(%rbp)
	movl	%esi, -108(%rbp)
	movl	%edx, -112(%rbp)
	movl	%ecx, -116(%rbp)
	movq	%rsp, %rax
	movq	%rax, %rbx
	movq	-104(%rbp), %rax
	movl	-108(%rbp), %edx
	movl	%edx, 4(%rax)
	movq	-104(%rbp), %rax
	movl	-112(%rbp), %edx
	movl	%edx, 8(%rax)
	movq	-104(%rbp), %rax
	movl	-116(%rbp), %edx
	movl	%edx, (%rax)
	movl	-108(%rbp), %eax
	imull	-112(%rbp), %eax
	movslq	%eax, %rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	movq	%rax, %rdi
	call	malloc
	movq	%rax, -64(%rbp)
	movq	-104(%rbp), %rax
	movq	-64(%rbp), %rdx
	movq	%rdx, 24(%rax)
	movl	-116(%rbp), %eax
	movslq	%eax, %rdx
	subq	$1, %rdx
	movq	%rdx, -72(%rbp)
	movslq	%eax, %rdx
	movq	%rdx, %r14
	movl	$0, %r15d
	movslq	%eax, %rdx
	movq	%rdx, %r12
	movl	$0, %r13d
	cltq
	salq	$3, %rax
	leaq	7(%rax), %rdx
	movl	$16, %eax
	subq	$1, %rax
	addq	%rdx, %rax
	movl	$16, %esi
	movl	$0, %edx
	divq	%rsi
	imulq	$16, %rax, %rax
	subq	%rax, %rsp
	movq	%rsp, %rax
	addq	$7, %rax
	shrq	$3, %rax
	salq	$3, %rax
	movq	%rax, -80(%rbp)
	movl	$0, -52(%rbp)
	jmp	.L75
.L76:
	movl	$16, %edi
	call	malloc
	movq	%rax, -88(%rbp)
	movq	-88(%rbp), %rax
	movq	-104(%rbp), %rdx
	movq	%rdx, (%rax)
	movq	-88(%rbp), %rax
	movl	-52(%rbp), %edx
	movl	%edx, 8(%rax)
	movl	-52(%rbp), %eax
	cltq
	leaq	0(,%rax,8), %rdx
	movq	-80(%rbp), %rax
	leaq	(%rdx,%rax), %rdi
	movq	-88(%rbp), %rax
	movq	%rax, %rcx
	movl	$_startRenderThread, %edx
	movl	$0, %esi
	call	pthread_create
	addl	$1, -52(%rbp)
.L75:
	movl	-52(%rbp), %eax
	cmpl	-116(%rbp), %eax
	jl	.L76
	movl	$0, -56(%rbp)
	jmp	.L77
.L78:
	movq	-80(%rbp), %rax
	movl	-56(%rbp), %edx
	movslq	%edx, %rdx
	movq	(%rax,%rdx,8), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	pthread_join
	addl	$1, -56(%rbp)
.L77:
	movl	-56(%rbp), %eax
	cmpl	-116(%rbp), %eax
	jl	.L78
	movl	-112(%rbp), %ecx
	movl	-108(%rbp), %edx
	movq	-64(%rbp), %rax
	movq	%rax, %rsi
	movl	$.LC10, %edi
	call	generateBmp
	movq	%rbx, %rsp
	nop
	leaq	-40(%rbp), %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE25:
	.size	renderScene, .-renderScene
	.section	.rodata
	.align 16
.LC1:
	.long	1073741824
	.long	1073741824
	.long	1073741824
	.long	1073741824
	.align 4
.LC2:
	.long	1082130432
	.align 16
.LC3:
	.long	2147483648
	.long	0
	.long	0
	.long	0
	.align 16
.LC4:
	.long	2147483648
	.long	2147483648
	.long	2147483648
	.long	2147483648
	.align 4
.LC5:
	.long	1065353216
	.align 4
.LC6:
	.long	1132396544
	.align 4
.LC7:
	.long	1073741824
	.align 8
.LC8:
	.long	0
	.long	1072693248
	.align 4
.LC9:
	.long	1070141433
	.ident	"GCC: (GNU) 6.0.142418.eb08b39 20151109 (experimental)"
	.section	.note.GNU-stack,"",@progbits
