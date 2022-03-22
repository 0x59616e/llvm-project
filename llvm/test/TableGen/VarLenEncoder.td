// RUN: llvm-tblgen -gen-emitter -I %p/../../include %s | FileCheck %s
// RUN: llvm-tblgen -gen-disassembler -I %p/../../include %s | \
// RUN:     FileCheck --check-prefix=CHECK-DECODE %s

// Check if VarLenCodeEmitterGen works correctly.

include "llvm/Target/Target.td"

def ArchInstrInfo : InstrInfo { }

def Arch : Target {
  let InstructionSet = ArchInstrInfo;
}

def Reg : Register<"reg">;

def RegClass : RegisterClass<"foo", [i64], 0, (add Reg)>;

def GR64 : RegisterOperand<RegClass>;

class MyMemOperand<dag sub_ops> : Operand<iPTR> {
  let MIOperandInfo = sub_ops;
  dag Base;
  dag Extension;
}

class MemOp16<string op_name> : MyMemOperand<(ops GR64:$reg, i16imm:$offset)> {
  // Testing sub-operand referencing.
  let Base = (operand "$"#op_name#".reg", 8);
  let Extension = (operand "$"#op_name#".offset", 16);
}

class MemOp32<string op_name> : MyMemOperand<(ops GR64:$reg, i32imm:$offset)> {
  let Base = (operand "$"#op_name#".reg", 8);
  // Testing variable-length instruction encoding.
  let Extension = (operand "$"#op_name#".offset", 32);
}

class MyVarInst<MyMemOperand memory_op> : Instruction {
  dag Inst;

  let OutOperandList = (outs GR64:$dst);
  let InOperandList  = (ins memory_op:$src);

  // Testing `ascend` and `descend`
  let Inst = (ascend
    (descend 0b10110111, memory_op.Base,
      !cond(
        !isa<MemOp16>(memory_op) : 0b0,
        !isa<MemOp32>(memory_op) : 0b1,
      )
    ),
    memory_op.Extension,
    // Testing operand referencing.
    (operand "$dst", 4),
    // Testing operand referencing with a certain bit range.
    (slice "$dst", 3, 1),
    // Testing custom encoder
    (operand "$dst", 2, (encoder "myCustomEncoder"))
  );
}

def FOO16 : MyVarInst<MemOp16<"src">>;
def FOO32 : MyVarInst<MemOp32<"src">>;

// The fixed bits part
// CHECK: {/*NumBits*/42,
// CHECK-SAME: // FOO16
// CHECK: {/*NumBits*/58,
// CHECK-SAME: // FOO32
// CHECK: UINT64_C(93696), // FOO16
// CHECK: UINT64_C(93697), // FOO32

// CHECK-LABEL: case ::FOO16: {
// CHECK: Scratch = Scratch.zextOrSelf(42);
// src.reg
// CHECK: getMachineOpValue(MI, MI.getOperand(1), /*Pos=*/1, Scratch, Fixups, STI);
// CHECK: Inst.insertBits(Scratch.extractBits(8, 0), 1);
// src.offset
// CHECK: getMachineOpValue(MI, MI.getOperand(2), /*Pos=*/17, Scratch, Fixups, STI);
// CHECK: Inst.insertBits(Scratch.extractBits(16, 0), 17);
// 1st dst
// CHECK: getMachineOpValue(MI, MI.getOperand(0), /*Pos=*/33, Scratch, Fixups, STI);
// CHECK: Inst.insertBits(Scratch.extractBits(4, 0), 33);
// 2nd dst
// CHECK: getMachineOpValue(MI, MI.getOperand(0), /*Pos=*/37, Scratch, Fixups, STI);
// CHECK: Inst.insertBits(Scratch.extractBits(3, 1), 37);
// dst w/ custom encoder
// CHECK: myCustomEncoder(MI, /*OpIdx=*/0, /*Pos=*/40, Scratch, Fixups, STI);
// CHECK: Inst.insertBits(Scratch.extractBits(2, 0), 40);

// CHECK-LABEL: case ::FOO32: {
// CHECK: Scratch = Scratch.zextOrSelf(58);
// src.reg
// CHECK: getMachineOpValue(MI, MI.getOperand(1), /*Pos=*/1, Scratch, Fixups, STI);
// CHECK: Inst.insertBits(Scratch.extractBits(8, 0), 1);
// src.offset
// CHECK: getMachineOpValue(MI, MI.getOperand(2), /*Pos=*/17, Scratch, Fixups, STI);
// CHECK: Inst.insertBits(Scratch.extractBits(32, 0), 17);
// 1st dst
// CHECK: getMachineOpValue(MI, MI.getOperand(0), /*Pos=*/49, Scratch, Fixups, STI);
// CHECK: Inst.insertBits(Scratch.extractBits(4, 0), 49);
// 2nd dst
// CHECK: getMachineOpValue(MI, MI.getOperand(0), /*Pos=*/53, Scratch, Fixups, STI);
// CHECK: Inst.insertBits(Scratch.extractBits(3, 1), 53);
// dst w/ custom encoder
// CHECK: myCustomEncoder(MI, /*OpIdx=*/0, /*Pos=*/56, Scratch, Fixups, STI);
// CHECK: Inst.insertBits(Scratch.extractBits(2, 0), 56);

// CHECK-DECODE: MCD::OPC_ExtractField, 0, 1,  // Inst{0} ...
// CHECK-DECODE-NEXT: MCD::OPC_FilterValue, 0, 12, 0, 0, // Skip to: 20
// CHECK-DECODE-NEXT: MCD::OPC_CheckField, 9, 8, 183, 1, 21, 0, 0, // Skip to: 37
// CHECK-DECODE-NEXT: MCD::OPC_Decode, 244, 1, 0, // Opcode: FOO16
// CHECK-DECODE-NEXT: MCD::OPC_FilterValue, 1, 12, 0, 0, // Skip to: 37
// CHECK-DECODE-NEXT: MCD::OPC_CheckField, 9, 8, 183, 1, 4, 0, 0, // Skip to: 37
// CHECK-DECODE-NEXT: MCD::OPC_Decode, 245, 1, 1, // Opcode: FOO32
// CHECK-DECODE-NEXT: MCD::OPC_Fail,

// Instruction length table
// CHECK-DECODE: 42,
// CHECK-DECODE-NEXT: 58,
// CHECK-DECODE-NEXT: };

// CHECK-DECODE-LABEL: case 0:
// CHECK-DECODE: tmp = 0x0;
// CHECK-DECODE-NEXT: insertBits(tmp, fieldFromInstruction(insn, 33, 4), 0, 4);
// CHECK-DECODE-NEXT: insertBits(tmp, fieldFromInstruction(insn, 37, 3), 1, 3);
// CHECK-DECODE-NEXT: insertBits(tmp, fieldFromInstruction(insn, 40, 2), 0, 2);
// CHECK-DECODE-NEXT: if (DecodeRegClassRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
// CHECK-DECODE-NEXT: tmp = fieldFromInstruction(insn, 1, 8);
// CHECK-DECODE-NEXT: if (DecodeRegClassRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
// CHECK-DECODE-NEXT: tmp = fieldFromInstruction(insn, 17, 16);
// CHECK-DECODE-NEXT: MI.addOperand(MCOperand::createImm(tmp));
// CHECK-DECODE-NEXT: return S;
// CHECK-DECODE-NEXT: case 1:
// CHECK-DECODE-NEXT: tmp = 0x0;
// CHECK-DECODE-NEXT: insertBits(tmp, fieldFromInstruction(insn, 49, 4), 0, 4);
// CHECK-DECODE-NEXT: insertBits(tmp, fieldFromInstruction(insn, 53, 3), 1, 3);
// CHECK-DECODE-NEXT: insertBits(tmp, fieldFromInstruction(insn, 56, 2), 0, 2);
// CHECK-DECODE-NEXT: if (DecodeRegClassRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
// CHECK-DECODE-NEXT: tmp = fieldFromInstruction(insn, 1, 8);
// CHECK-DECODE-NEXT: if (DecodeRegClassRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
// CHECK-DECODE-NEXT: tmp = fieldFromInstruction(insn, 17, 32);
// CHECK-DECODE-NEXT: MI.addOperand(MCOperand::createImm(tmp));
// CHECK-DECODE-NEXT: return S;

// CHECK-DECODE-LABEL: case MCD::OPC_ExtractField: {
// CHECK-DECODE: makeUp(insn, Start + Len);

// CHECK-DECODE-LABEL: case MCD::OPC_CheckField: {
// CHECK-DECODE: makeUp(insn, Start + Len);

// CHECK-DECODE-LABEL: case MCD::OPC_Decode: {
// CHECK-DECODE: Len = InstrLenTable[Opc];
// CHECK-DECODE-NEXT: makeUp(insn, Len);
