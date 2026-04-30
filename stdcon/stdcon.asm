
libstdcon.so:     формат файла elf64-x86-64


Дизассемблирование раздела .init:

0000000000001000 <.init>:
    1000:	48 83 ec 08          	sub    $0x8,%rsp
    1004:	48 8b 05 c5 2f 00 00 	mov    0x2fc5(%rip),%rax        # 3fd0 <__gmon_start__@Base>
    100b:	48 85 c0             	test   %rax,%rax
    100e:	74 02                	je     1012 <printf@plt-0x1e>
    1010:	ff d0                	call   *%rax
    1012:	48 83 c4 08          	add    $0x8,%rsp
    1016:	c3                   	ret

Дизассемблирование раздела .plt:

0000000000001020 <printf@plt-0x10>:
    1020:	ff 35 ca 2f 00 00    	push   0x2fca(%rip)        # 3ff0 <println@@Base+0x2ee0>
    1026:	ff 25 cc 2f 00 00    	jmp    *0x2fcc(%rip)        # 3ff8 <println@@Base+0x2ee8>
    102c:	0f 1f 40 00          	nopl   0x0(%rax)

0000000000001030 <printf@plt>:
    1030:	ff 25 ca 2f 00 00    	jmp    *0x2fca(%rip)        # 4000 <printf@GLIBC_2.2.5>
    1036:	68 00 00 00 00       	push   $0x0
    103b:	e9 e0 ff ff ff       	jmp    1020 <printf@plt-0x10>

Дизассемблирование раздела .plt.got:

0000000000001040 <__cxa_finalize@plt>:
    1040:	ff 25 9a 2f 00 00    	jmp    *0x2f9a(%rip)        # 3fe0 <__cxa_finalize@GLIBC_2.2.5>
    1046:	66 90                	xchg   %ax,%ax

Дизассемблирование раздела .text:

0000000000001050 <println@@Base-0xc0>:
    1050:	48 8d 3d b9 2f 00 00 	lea    0x2fb9(%rip),%rdi        # 4010 <println@@Base+0x2f00>
    1057:	48 8d 05 b2 2f 00 00 	lea    0x2fb2(%rip),%rax        # 4010 <println@@Base+0x2f00>
    105e:	48 39 f8             	cmp    %rdi,%rax
    1061:	74 15                	je     1078 <__cxa_finalize@plt+0x38>
    1063:	48 8b 05 5e 2f 00 00 	mov    0x2f5e(%rip),%rax        # 3fc8 <println@@Base+0x2eb8>
    106a:	48 85 c0             	test   %rax,%rax
    106d:	74 09                	je     1078 <__cxa_finalize@plt+0x38>
    106f:	ff e0                	jmp    *%rax
    1071:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    1078:	c3                   	ret
    1079:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    1080:	48 8d 3d 89 2f 00 00 	lea    0x2f89(%rip),%rdi        # 4010 <println@@Base+0x2f00>
    1087:	48 8d 35 82 2f 00 00 	lea    0x2f82(%rip),%rsi        # 4010 <println@@Base+0x2f00>
    108e:	48 29 fe             	sub    %rdi,%rsi
    1091:	48 89 f0             	mov    %rsi,%rax
    1094:	48 c1 ee 3f          	shr    $0x3f,%rsi
    1098:	48 c1 f8 03          	sar    $0x3,%rax
    109c:	48 01 c6             	add    %rax,%rsi
    109f:	48 d1 fe             	sar    $1,%rsi
    10a2:	74 14                	je     10b8 <__cxa_finalize@plt+0x78>
    10a4:	48 8b 05 2d 2f 00 00 	mov    0x2f2d(%rip),%rax        # 3fd8 <println@@Base+0x2ec8>
    10ab:	48 85 c0             	test   %rax,%rax
    10ae:	74 08                	je     10b8 <__cxa_finalize@plt+0x78>
    10b0:	ff e0                	jmp    *%rax
    10b2:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    10b8:	c3                   	ret
    10b9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    10c0:	f3 0f 1e fa          	endbr64
    10c4:	80 3d 45 2f 00 00 00 	cmpb   $0x0,0x2f45(%rip)        # 4010 <println@@Base+0x2f00>
    10cb:	75 2b                	jne    10f8 <__cxa_finalize@plt+0xb8>
    10cd:	55                   	push   %rbp
    10ce:	48 83 3d 0a 2f 00 00 	cmpq   $0x0,0x2f0a(%rip)        # 3fe0 <println@@Base+0x2ed0>
    10d5:	00 
    10d6:	48 89 e5             	mov    %rsp,%rbp
    10d9:	74 0c                	je     10e7 <__cxa_finalize@plt+0xa7>
    10db:	48 8b 3d 26 2f 00 00 	mov    0x2f26(%rip),%rdi        # 4008 <println@@Base+0x2ef8>
    10e2:	e8 59 ff ff ff       	call   1040 <__cxa_finalize@plt>
    10e7:	e8 64 ff ff ff       	call   1050 <__cxa_finalize@plt+0x10>
    10ec:	c6 05 1d 2f 00 00 01 	movb   $0x1,0x2f1d(%rip)        # 4010 <println@@Base+0x2f00>
    10f3:	5d                   	pop    %rbp
    10f4:	c3                   	ret
    10f5:	0f 1f 00             	nopl   (%rax)
    10f8:	c3                   	ret
    10f9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    1100:	f3 0f 1e fa          	endbr64
    1104:	e9 77 ff ff ff       	jmp    1080 <__cxa_finalize@plt+0x40>
    1109:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

0000000000001110 <println@@Base>:
    1110:	48 89 fe             	mov    %rdi,%rsi
    1113:	31 c0                	xor    %eax,%eax
    1115:	48 8d 3d e4 0e 00 00 	lea    0xee4(%rip),%rdi        # 2000 <println@@Base+0xef0>
    111c:	e9 0f ff ff ff       	jmp    1030 <printf@plt>

Дизассемблирование раздела .fini:

0000000000001124 <.fini>:
    1124:	48 83 ec 08          	sub    $0x8,%rsp
    1128:	48 83 c4 08          	add    $0x8,%rsp
    112c:	c3                   	ret
