//===-- M68kLegalizerInfo.cpp -----------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// This file implements the targeting of the Machinelegalizer class for M68k.
//===----------------------------------------------------------------------===//

#include "M68kLegalizerInfo.h"
#include "M68kMachineFunction.h"
#include "llvm/CodeGen/GlobalISel/LegalizerHelper.h"
#include "llvm/CodeGen/GlobalISel/LegalizerInfo.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/TargetOpcodes.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Alignment.h"

using namespace llvm;

M68kLegalizerInfo::M68kLegalizerInfo(const M68kSubtarget &ST) {
  using namespace TargetOpcode;
  const LLT s1 = LLT::scalar(1);
  const LLT s8 = LLT::scalar(8);
  const LLT s16 = LLT::scalar(16);
  const LLT s32 = LLT::scalar(32);
  const LLT p0 = LLT::pointer(0, 32);

  getActionDefinitionsBuilder({G_ADD, G_SUB, G_MUL, G_UDIV, G_AND})
      .legalFor({s32});

  getActionDefinitionsBuilder(G_CONSTANT).legalFor({s32, p0});

  getActionDefinitionsBuilder({G_FRAME_INDEX, G_GLOBAL_VALUE}).legalFor({p0});

  getActionDefinitionsBuilder({G_ADD, G_SUB, G_MUL, G_UDIV, G_AND})
      .legalFor({s8, s16, s32})
      .clampScalar(0, s8, s32)
      .widenScalarToNextPow2(0, 8);

  getActionDefinitionsBuilder(G_CONSTANT)
      .legalFor({s32, p0})
      .clampScalar(0, s32, s32);

  getActionDefinitionsBuilder({G_FRAME_INDEX, G_GLOBAL_VALUE}).legalFor({p0});

  getActionDefinitionsBuilder({G_STORE, G_LOAD})
      .legalForTypesWithMemDesc({{s32, p0, s32, 4},
                                 {s32, p0, s16, 4},
                                 {s32, p0, s8, 4},
                                 {s16, p0, s16, 2},
                                 {s8, p0, s8, 1},
                                 {p0, p0, s32, 4}})
      .clampScalar(0, s8, s32);

  getActionDefinitionsBuilder(G_PTR_ADD).legalFor({{p0, s32}});
  getActionDefinitionsBuilder(G_PHI).legalFor({p0, s32});
  getActionDefinitionsBuilder(G_BRCOND).legalFor({s1});
  getActionDefinitionsBuilder(G_ICMP)
      .legalFor({{s32, s32}, {s32, p0}})
      .clampScalar(0, s32, s32);
  getActionDefinitionsBuilder(G_SELECT).legalFor({{s32, s1}});

  getActionDefinitionsBuilder({G_SHL, G_ASHR}).legalFor({s32, s32});

  getActionDefinitionsBuilder(G_UADDO).legalFor({{s32, s1}});

  getActionDefinitionsBuilder(G_VAARG).customForCartesianProduct({s8, s16, s32},
                                                                 {p0});

  getActionDefinitionsBuilder(G_VASTART).customFor({p0});

  getLegacyLegalizerInfo().computeTables();
}

bool M68kLegalizerInfo::legalizeCustom(LegalizerHelper &Helper,
                                       MachineInstr &MI) const {
  switch (MI.getOpcode()) {
  default:
    return false;
  case TargetOpcode::G_VAARG:
    return legalizeVaArg(Helper, MI);
  case TargetOpcode::G_VASTART:
    return legalizeVaStart(Helper, MI);
  }
}

bool M68kLegalizerInfo::legalizeVaArg(LegalizerHelper &Helper,
                                      MachineInstr &MI) const {
  MachineFunction &MF = *MI.getMF();
  MachineIRBuilder &MIRBuilder = Helper.MIRBuilder;
  MachineRegisterInfo &MRI = *MIRBuilder.getMRI();

  MachineOperand &DstOp = MI.getOperand(0);
  MachineOperand &ArgPtrPtr = MI.getOperand(1);
  auto MinAlign = Align(MI.getOperand(2).getImm());

  LLT p0 = LLT::pointer(0, 32);
  LLT s32 = LLT::scalar(32);
  LLT DstTy = MRI.getType(DstOp.getReg());
  auto DstAlign = Align(DstTy.getSizeInBits() / 8);
  if (DstAlign < MinAlign)
    DstAlign = MinAlign;

  auto ArgPtr = MIRBuilder.buildLoad(
      p0, ArgPtrPtr,
      *MF.getMachineMemOperand(MachinePointerInfo(), MachineMemOperand::MOLoad,
                               p0, Align(4)));
  auto Offset = MIRBuilder.buildConstant(s32, APInt(32, DstAlign.value()));
  auto NextArgPtr =
      MIRBuilder.buildPtrAdd(p0, ArgPtr.getReg(0), Offset.getReg(0));
  MIRBuilder.buildStore(NextArgPtr.getReg(0), ArgPtrPtr,
                        *MF.getMachineMemOperand(MachinePointerInfo(),
                                                 MachineMemOperand::MOLoad, p0,
                                                 Align(4)));
  MIRBuilder.buildLoad(DstOp.getReg(), ArgPtr.getReg(0),
                       *MF.getMachineMemOperand(MachinePointerInfo(),
                                                MachineMemOperand::MOLoad, p0,
                                                Align(4)));
  MI.eraseFromParent();
  return true;
}

bool M68kLegalizerInfo::legalizeVaStart(LegalizerHelper &Helper,
                                        MachineInstr &MI) const {
  MachineFunction &MF = *MI.getMF();
  MachineIRBuilder &MIRBuilder = Helper.MIRBuilder;
  M68kMachineFunctionInfo *FuncInfo = MF.getInfo<M68kMachineFunctionInfo>();

  LLT p0 = LLT::pointer(0, 32);
  auto VaAddr = MIRBuilder.buildFrameIndex(p0, FuncInfo->getVarArgsFrameIndex());
  MIRBuilder.buildStore(VaAddr.getReg(0), MI.getOperand(0).getReg(),
                        *MF.getMachineMemOperand(MachinePointerInfo(),
                                                 MachineMemOperand::MOStore, p0,
                                                 Align(4)));
  MI.eraseFromParent();
  return true;
}
