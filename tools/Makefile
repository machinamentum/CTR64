all : ctrx_disasm ctrx_hsf ctr64_z64

clean:
	@make -f MakefileDisasm clean
	@make -f MakefileHsf clean
	@make -f MakefileZ64 clean

ctrx_disasm:
	make -f MakefileDisasm

ctrx_hsf:
	make -f MakefileHsf

ctr64_z64:
	make -f MakefileZ64