#include <algorithm>
#include <cassert>
#include <iterator>
#include <memory>
#include <set>
#include <utility>
#include <vector>
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "map"

using namespace llvm;

namespace {

#define DEBUG_TYPE "lva"
class LVAPass : public FunctionPass {
	std::map<Value *, std::set<Value *>> Gen, Kill;
	std::map<Value *, std::set<Value *>> In, Out;

       public:
	static char ID;
	LVAPass() : FunctionPass(ID) {}
	void getGen(Instruction &I) {
		if (StoreInst *SI = dyn_cast<StoreInst>(&I)) {
			Value *Val = SI->getValueOperand();
			if (isa<Instruction>(Val)) Gen[&I].insert(Val);
			Value *Value = SI->getPointerOperand();
			if (isa<Instruction>(Value)) Gen[&I].insert(Value);
		}
		if (LoadInst *LI = dyn_cast<LoadInst>(&I)) {
			Value *Ptr = LI->getPointerOperand();
			if (isa<Instruction>(Ptr)) Gen[&I].insert(Ptr);
		}
		if (ReturnInst *RI = dyn_cast<ReturnInst>(&I)) {
			Value *RetValue = RI->getReturnValue();
			if (isa<Instruction>(RetValue))
				Gen[&I].insert(RetValue);
		}
		if (I.isBinaryOp()) {
			Gen[&I].insert(I.getOperand(0));
			Gen[&I].insert(I.getOperand(1));
		}
	}
	void printGen(Instruction &I) {
		errs() << "Gen set for: " << I << "\n";
		for (auto Inst : Gen[&I]) {
			errs() << *Inst << "\n";
		}
	}
	void printKill(Instruction &I) {
		errs() << "Kill set for: " << I << "\n";
		for (auto Inst : Kill[&I]) {
			errs() << *Inst << "\n";
		}
	}
	void getKill(Instruction &I) {
		if (AllocaInst *AI = dyn_cast<AllocaInst>(&I)) {
			Kill[&I].insert(&I);
		}
		if (StoreInst *SI = dyn_cast<StoreInst>(&I)) {
		}
		if (LoadInst *LI = dyn_cast<LoadInst>(&I)) {
			Kill[&I].insert(&I);
		}
		if (I.isBinaryOp()) {
			Kill[&I].insert(&I);
		}
	}
	void getOut(Instruction &I) {
		if (I.isTerminator()) {
			for (BasicBlock *Succ : successors(I.getParent())) {
				Instruction *Inst = &*(Succ->begin());
				Out[&I].insert(In[Inst].begin(),
					       In[Inst].end());
			}
		} else {
			Instruction *Next = I.getNextNonDebugInstruction();
			Out[&I] = In[Next];
		}
	}
	void getIn(Instruction &I) {
		std::set<Value *> Temp = Out[&I];
		for (auto Inst : Kill[&I]) {
			Temp.erase(Inst);
		}
		for (auto Inst : Gen[&I]) {
			Temp.insert(Inst);
		}
		In[&I] = Temp;
	}
	void printIn(Instruction &I) {
		errs() << "In set for: " << I << "\n";
		for (auto Inst : In[&I]) {
			errs() << *Inst << "\n";
		}
	}
	void printOut(Instruction &I) {
		errs() << "Out set for: " << I << "\n";
		for (auto Inst : Out[&I]) {
			errs() << *Inst << "\n";
		}
	}
	bool runOnFunction(Function &F) override {
		inst_iterator StartInst = inst_begin(F);
		inst_iterator EndInst = inst_end(F);
		do {
			EndInst--;
			getGen(*EndInst);
			getKill(*EndInst);
			// printKill(*EndInst);
			// printGen(*EndInst);
			getOut(*EndInst);
			getIn(*EndInst);
			// printOut(*EndInst);
			printIn(*EndInst);
		} while (StartInst != EndInst);
		return false;
	}
};  // namespace
}  // namespace
char LVAPass::ID = 0;
static RegisterPass<LVAPass> X("lva", "Live variable analysis", true, true);
