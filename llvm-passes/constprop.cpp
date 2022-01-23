#define DEBUG_TYPE "ConstProp"
#include "utils.h"

namespace {
    class ConstProp : public FunctionPass {
    public:
        static char ID;
        ConstProp() : FunctionPass(ID) {}
        virtual bool runOnFunction(Function &F) override;
    };
}

bool calculate(int operand, int rhs, int lhs, int &result) {
    switch(operand) {
        case Instruction::Add:
            result = lhs + rhs;
            break;
        case Instruction::Sub:
            result = lhs - rhs;
            break;
        case Instruction::Mul:
            result = lhs * rhs;
            break;
        case Instruction::AShr:
            result = lhs >> rhs;
            break;
        case Instruction::Shl:
            result = lhs << rhs;
            break;
        default:
            return false;
            break;
    } 
    return true;
}


bool ConstProp::runOnFunction(Function &F) {
    SmallVector<ConstantInt*, 2> consts;
    SmallVector<Instruction*, 2> insts; 
    
    bool IRchanged = false;
    for (BasicBlock &BB : F) {
        for (Instruction &I : BB) {
            if (dyn_cast<BinaryOperator>(&I)) {  
                for (Use &op : I.operands()) {
                    if (dyn_cast<ConstantInt>(op)) {
                        consts.push_back(dyn_cast<ConstantInt>(op)); 
                    } else if (dyn_cast<Instruction>(op)){
                        insts.push_back(dyn_cast<Instruction>(op));
                    } 
                }

                if (consts.size() == 2) {
                    int rightside = consts[consts.size() - 1]->getSExtValue();
                    int leftside = consts[consts.size() - 2]->getSExtValue();
                    int result;

                    if (calculate(I.getOpcode(), rightside, leftside, result)) {
                        ConstantInt *retrievedConstInt = ConstantInt::get(consts.back()->getType(), result);
                        consts.pop_back(); 

                        if (dyn_cast<Value>(retrievedConstInt)) { 
                            I.replaceAllUsesWith(retrievedConstInt);
                            IRchanged = true;
                        } 
                    }
                } else if (consts.size() == 2 && insts.size() == 1) { 
                    int op = I.getOpcode();
                    
                    if ((op == Instruction::Add && consts.back()->getSExtValue() == 0) ||  
                        (op == Instruction::Sub && consts.back()->getSExtValue() == 0) ||
                        (op == Instruction::Mul && consts.back()->getSExtValue() == 1)) {
                        I.replaceAllUsesWith(insts.back()); 
                        insts.pop_back();
                    }
                    consts.pop_back(); 
                } 
                consts.clear();
                insts.clear();
            }
        }
    }
    return IRchanged;
}
 
char ConstProp::ID = 0;
static RegisterPass<ConstProp> X("coco-constprop", "ConstProp");