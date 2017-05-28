// Context switch
//
//     void swtch(unsigned **oldsp, unsigned **newsp);

.globl swtch
swtch:
	pushl %ebp
	mov %esp,%ebp	
	pushfl
	pusha
	
	mov 8(%ebp),%eax	#EAX=oldsp
	mov %esp,(%eax)		#actualiza SP en old
	mov 12(%ebp),%eax	#EAX=newsp
	mov (%eax),	%esp	#SP=*newsp
	
	popa
	popfl
	popl %ebp
	ret
