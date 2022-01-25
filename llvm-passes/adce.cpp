#define DEBUG_TYPE "ADCE"
#include "utils.h"
#include <iostream>
using namespace std;

namespace {
    class ADCE : public FunctionPass {
    public: 
        static char ID;
        ADCE() : FunctionPass(ID) {}
        virtual bool runOnFunction(Function &F) override; 

        bool IRchanged = false;
        SmallVector<Instruction*, 256> WorkList; // idk about the size though, in pdf it says "try to estimate an upper bound for your data structure"
        DenseMap<Instruction*, bool> LiveSet;
    };
}

bool ADCE::runOnFunction(Function &F) { 
    LiveSet.clear(); // LiveSet = emptySet

    for (BasicBlock *BB: depth_first(&F)) { 
        for (Instruction &I : *BB) {           
            if (I.mayHaveSideEffects() || I.isTerminator() ||
               (dyn_cast<LoadInst>(&I) && dyn_cast<LoadInst>(&I)->isVolatile()) ||
                dyn_cast<StoreInst>(&I) || dyn_cast<CallInst>(&I)) { // is trivially live (if it has side effects) 
                LiveSet.insert({&I, true}); // mark live
                WorkList.push_back(&I); 
            } else if (I.use_empty()) {
                I.dropAllReferences(); // remove I from BB
                IRchanged = true;
            }
        }
    }
  
    while (!WorkList.empty()){
        Instruction *I = WorkList.back();
        WorkList.pop_back(); 

        if (LiveSet.find(I) != LiveSet.end()) { // if BB containing I is reachable
            for (auto const &op : I->operands()) { 
                if (dyn_cast<Instruction>(op)) { // if operand is an instruction
                    Instruction *I_op = dyn_cast<Instruction>(op);
                    LiveSet.insert({I_op, true}); // mark live
                    WorkList.push_back(I_op);
                }
            }
        }
    }
    
    // Delete instructions that is not in LiveSet
    for (BasicBlock *BB : depth_first(&F)) {
        for (Instruction &I : *BB) { 
            if (LiveSet.find(&I) == LiveSet.end()) { 
                LiveSet.insert({&I, false});  
                IRchanged = true;
                I.dropAllReferences();
            }
        }
    }    

    for ( const auto &I : LiveSet ) {
        if (I.second == false)
            I.first->eraseFromParent();
    }  

    return IRchanged;
}

char ADCE::ID = 0;
static RegisterPass<ADCE> X("coco-adce", "ADCE"); 