#define DEBUG_TYPE "LICM"
#include "utils.h"

using namespace std;

namespace {
    class LICM : public LoopPass {
    public:
        static char ID;
        LICM() : LoopPass(ID) {}
        virtual bool runOnLoop(Loop *L, LPPassManager &LPM) override;
        void getAnalysisUsage(AnalysisUsage &AU) const override;
    };
}
 
void LICM::getAnalysisUsage(AnalysisUsage &AU) const {
    // Tell LLVM we need some analysis info which we use for analyzing the
    // DominatorTree.
    AU.setPreservesCFG();
    AU.addRequired<LoopInfoWrapperPass>();
    getLoopAnalysisUsage(AU);
}   
 
bool _isLoopInvariant(Instruction *I, BasicBlock *BB, Loop *L, const DominatorTree *DT) {
    bool result = false;

    // It is a binary operator, shift, select, cast, getelementptr.
    if (dyn_cast<ConstantExpr>(I) || (!I || DT->properlyDominates(I->getParent(), L->getHeader()) ||
        I -> isBinaryOp() || I -> isShift() || dyn_cast<SelectInst>(I) || I -> isCast() || dyn_cast<GetElementPtrInst>(I))) {
        for (Use &U : I->operands()) {
            Instruction *Inst = dyn_cast<Instruction>(U);

            // All the operands of the instruction are loop invariant.
            // That is, every operand of the instruction is either a constant or computed outside the loop.
            if (dyn_cast<ConstantExpr>(U) || (!Inst || DT->properlyDominates(Inst->getParent(), L->getHeader()) ||
                Inst -> isBinaryOp() || Inst -> isShift() || dyn_cast<SelectInst>(Inst) || Inst -> isCast() || dyn_cast<GetElementPtrInst>(Inst))) {
                result = true;
            } else { 
                return false;
            } 
        }
    }
     
    return result;
}

bool safe_to_hoist(Instruction *I, BasicBlock *BB, Loop *L, const DominatorTree *DT) {
    // It has no side effects 
    if (!(I -> mayHaveSideEffects()))
        return true;

    // The basic block containing the instruction dominates all exit blocks for the loop.
    SmallVector<BasicBlock*, 8> ExitingBlocks;
    L->getExitingBlocks(ExitingBlocks);

    bool result = true;
    for (BasicBlock *eBB : ExitingBlocks) {
        if (!DT->properlyDominates(BB, eBB)) {
            return false;
            break;
        }
    } 

    if (result) {
        return true;
    }

    return false;
}
 
bool LICM::runOnLoop(Loop *L, LPPassManager &LPM) { 
    BasicBlock *currentLoopHeader = L->getHeader(); // Obtain the current loop header.
    DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
 
    for (BasicBlock *BB : L->blocks()) {
        if (DT->dominates(currentLoopHeader, BB) && // each basic block BB dominated by loop header
            DT->isReachableFromEntry(BB)) {         // not in an inner loop or outside loop
                
            BasicBlock::iterator next = BB->begin();
   
            while (next != BB->end()) {
                Instruction *I = dyn_cast<Instruction>(next++);
                 
                // move every instruction that is loop invariant and is safe to hoist
                if (_isLoopInvariant(I, BB, L, DT) && safe_to_hoist(I, BB, L, DT)) {
                    BasicBlock *pre = L->getLoopPreheader();
                    Instruction *term = pre->getTerminator(); 
                    I->moveBefore(term);
                    I->dropUnknownNonDebugMetadata();
                } 
            }
        }
    }

    return false;
}

char LICM::ID = 0;
RegisterPass<LICM> X("coco-licm", "LICM");