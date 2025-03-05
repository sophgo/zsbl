	.global pka0_fw_begin
	.global pka0_fw_end
	.section .data.pka_fw
pka0_fw_begin:
	.incbin "elapka.elf"
pka0_fw_end:
