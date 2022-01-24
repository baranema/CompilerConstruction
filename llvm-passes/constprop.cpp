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

bool calculate(int operand, int rightside, int leftside, int &result) {
    // operations that can be simplified (example: additions where one operand is a 0)
    if (rightside == 0 && (operand == Instruction::Sub || operand == Instruction::Shl ||
        operand == Instruction::AShr || operand == Instruction::Add)) {
        result = leftside; 
    } 
    else if (leftside == 0 && operand == Instruction::Add) { 
        result = rightside;
    } 
    else if (rightside == 1 && operand == Instruction::Mul) {
        result = leftside;
    } 
    else if (leftside == 1 && operand == Instruction::Mul) {
        result = rightside;
    } 
    else if ((rightside == 0 && operand == Instruction::Mul) || (rightside == 0 && operand == Instruction::AShr)) {
        result = rightside;
    } 
    else if ((leftside == 0 && operand == Instruction::Mul) || (leftside == 0 && operand == Instruction::Shl)) {
        result = leftside;
    } else {
        switch (operand) {
            case Instruction::Add:
                result = leftside + rightside;
                break;
            case Instruction::Sub:
                result = leftside - rightside;
                break;
            case Instruction::Mul:
                result = leftside * rightside;
                break;
            case Instruction::AShr:
                result = leftside >> rightside;
                break;
            case Instruction::Shl:
                result = leftside << rightside;
                break;
            default:
                return false;
                break;
        } 
    }
    return true;
}

bool ConstProp::runOnFunction(Function &F) {
    SmallVector<ConstantInt*, 2> C; 
    bool IRchanged = false;

    for (BasicBlock &BB : F) {
        for (Instruction &I : BB) {
            if (dyn_cast<BinaryOperator>(&I)) { // if instruction is binary operation 
                for (Use &op : I.operands()) { 
                    if (dyn_cast<ConstantInt>(op)) { // is operand is a constant
                        C.push_back(dyn_cast<ConstantInt>(op)); 
                    }  
                }
 
                if (C.size() == 2) {
                    int rightside = C[C.size() - 1]->getSExtValue();
                    int leftside = C[C.size() - 2]->getSExtValue();
                    int calculatedResult;
                    ConstantInt *retrievedConstInt = nullptr;

                //    LOG_LINE(leftside << " " << rightside);
 
                    if (calculate(I.getOpcode(), rightside, leftside, calculatedResult)) {
                        retrievedConstInt = ConstantInt::get(C.back()->getType(), calculatedResult);
                        C.pop_back();
                        if (dyn_cast<Value>(retrievedConstInt)) { 
                            I.replaceAllUsesWith(retrievedConstInt);
                            IRchanged = true;
                        } 
                    }
                }
                C.clear(); 
            }
        }
    }
    return IRchanged;
}
 
char ConstProp::ID = 0;
static RegisterPass<ConstProp> X("coco-constprop", "ConstProp");