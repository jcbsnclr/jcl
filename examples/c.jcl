import asm;

let add [asm.assemble {
	args 1;

	push 2;
	add;
	return;
}];

add [ad ]