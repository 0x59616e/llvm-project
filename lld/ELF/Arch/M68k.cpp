#include "Symbols.h"
#include "SyntheticSections.h"
#include "Target.h"
#include "Thunks.h"
#include "lld/Common/ErrorHandler.h"
#include "llvm/Object/ELF.h"
#include "llvm/Support/Endian.h"

using namespace llvm;
using namespace lld;
using namespace lld::elf;

namespace {
class M68k : public TargetInfo {
public:
  M68k() { /* do nothing */
  }
  RelExpr getRelExpr(RelType type, const Symbol &s,
                     const uint8_t *loc) const override;
  void relocate(uint8_t *loc, const Relocation &rel,
                uint64_t val) const override;
};
} // namespace

static TargetInfo *getTargetInfo() {
  static M68k t;
  return &t;
}

RelExpr M68k::getRelExpr(RelType type, const Symbol &s,
                         const uint8_t *loc) const {
  llvm_unreachable("unimplemented");
}

void M68k::relocate(uint8_t *loc, const Relocation &rel, uint64_t val) const {
  llvm_unreachable("unimplemented");
}

TargetInfo *elf::getM68kTargetInfo() { return getTargetInfo(); }
