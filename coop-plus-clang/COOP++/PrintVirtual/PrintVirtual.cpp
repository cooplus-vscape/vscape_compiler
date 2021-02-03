//===- PrintVirtual.cpp ---------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Example clang plugin which simply prints the names of all the top-level decls
// in the input file.
//
//===----------------------------------------------------------------------===//

#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Sema/Sema.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/SourceLocation.h"
using namespace clang;



namespace {

// Helper class for this plugin
// FIXME: should not use public data members
class PrintVirtualOptions{
public:
  unsigned printCallSite : 1;
  unsigned printVirtualFn : 1;
  unsigned printVFMember : 1;
  unsigned printClassLayout : 1;
};

// Helper class to represent a Decl* uniquely.
// Ideally, we can use CodeGenModule's getMangledName(GlobalDecl*) to compute a unique name.
// But, we don't have CodeGenModule yet, since it works on AST directly.
class CustomizedMangle{
public:
  CustomizedMangle(const NamedDecl* const d, ASTContext *Context){
    SourceLocation Loc = d->getLocStart();
    name = d->getName().data();

    if(Loc.isValid())
    {
      valid = true;
      SourceManager &SM = Context->getSourceManager();
      PresumedLoc PLoc = SM.getPresumedLoc(Loc);
      file = PLoc.getFilename();
      line = PLoc.getLine();
      column = PLoc.getColumn();
    }
    else
      valid = false;
  }
  void print(raw_ostream& os){
    os << "Name: \t" << name << "\n";
    if(valid)
      os << "\tLoc:  \t(" << file << ", " << line << ", " << column << ")\n";
    else
      os << "\tLoc:  \t(NULL)\n";
  }
private:
  const char* name;
  bool valid;
  const char* file;
  unsigned line;
  unsigned column;
};

raw_ostream& operator<<(raw_ostream& Out, CustomizedMangle &m){
  m.print(Out);
  return Out;
}

// RecursiveASTVisitor interface provided by Clang. 
// 
// Clients can call functions like TraverseDecl(), TraverseStmt(), TraverseAttr(), TraverseType(), and so on, to drive the RecursiveASTVisitor.
// The RecursiveASTVisitor provides a determined traverse algorithm on AST nodes, and call Visit* interface on each proper node.
//
// Users should customize their own Visit* interface, to do their own work, by inheriting from RecursiveASTVisitor.
//    Note: Visit* functions are not virtual, RecursiveASTVisitor will first convert itself to the dervied class, and call its overloaded function.
//          e.g., getDerived().Traverse##CLASS##Decl(static_cast<CLASS##Decl *>(D)))
class PrintVirtualVisitor
  : public RecursiveASTVisitor<PrintVirtualVisitor> {
public:
  explicit PrintVirtualVisitor(ASTContext *Context, PrintVirtualOptions* opts)
    : Context(Context), opts(opts) {}

  bool VisitCXXRecordDecl(CXXRecordDecl *Declaration) {
    if(opts->printClassLayout) {
      llvm::coop_outs_cls() << "\nClass ";
      CustomizedMangle mangled(Declaration, Context);
      llvm::coop_outs_cls() << mangled;
      llvm::coop_outs_cls()<< "\nClass layout: \n";
      //Context->DumpRecordLayout(Declaration, llvm::coop_outs_cls()); // Class layout is not ready right now!!! 
    }
    return true;
  }

  bool VisitCXXMethodDecl(CXXMethodDecl *Declaration) {
    if(opts->printVirtualFn) {
      if(Declaration->isVirtual()) { //  Declaration->size_overridden_methods())
        llvm::coop_outs_vf() << "\nVirtual function ";
        CustomizedMangle mangled(Declaration, Context);
        llvm::coop_outs_vf() << mangled;

        llvm::coop_outs_vf() << "\nParent class ";
        CXXRecordDecl* rd = Declaration->getParent();
        CustomizedMangle mangled2(rd, Context);
        llvm::coop_outs_vf() << mangled2;

        llvm::coop_outs_vf() << "\nOverrided functions:";
        for(auto i = Declaration->begin_overridden_methods(), e = Declaration->end_overridden_methods(); i != e; i++) {
          CustomizedMangle mangled(*i, Context);
          llvm::coop_outs_vf() << mangled;
        }
      }
    }
    return true;
  } 

  bool VisitCXXMemberCallExpr(CXXMemberCallExpr* Expr) {
    if(opts->printCallSite) {
      CXXMethodDecl*  Declaration = Expr->getMethodDecl();

      if(Declaration->isVirtual()) { 
        llvm::coop_outs_cs() << "\nVirtual call site:\n\t";
        SourceLocation Loc = Expr->getLocStart();
        if(Loc.isValid())
          Loc.print(llvm::coop_outs_cs(), Context->getSourceManager());
        else
          llvm::coop_outs_cs() << "(NULL)";

        llvm::coop_outs_cs() << "\nTarget virtual function ";
        CustomizedMangle mangled(Declaration, Context);
        mangled.print(llvm::coop_outs_cs());

      }
    }
    return true;
  }
private:
  ASTContext *Context;
  PrintVirtualOptions* opts;
};

// Meat of a plugin: ASTConsumer, defines what you want to do for each AST node.
//    : HandleTranslationUnit(), will be called when the entire translation unit has been parsed 
//
// There are other Handle** interfaces can be overrided, e.g., HandleTopLevelDecl 
//
// Here, we utilize RecursiveASTVisitor to help visit all AST nodes.
class PrintVirtualConsumer : public ASTConsumer {
  CompilerInstance &Instance;

public:
  PrintVirtualConsumer(CompilerInstance &Instance,
                         PrintVirtualOptions* opts,
                         ASTContext &Context)
      : Instance(Instance), Visitor(&Context, opts) {}

  void HandleTranslationUnit(ASTContext& Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

  PrintVirtualVisitor Visitor;
};


// The PluginASTAction to be scheduled by Clang 
//    : ParseArgs(), accepts the argument from command line
//    : CreateASTConsumer(), returns an ASTConsumer to Clang.
class PrintVirtualAction : public PluginASTAction {
  PrintVirtualOptions* opts;
protected:
  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &args) override {

    opts = new PrintVirtualOptions();

    for (unsigned i = 0, e = args.size(); i != e; ++i) {
      llvm::errs() << "PrintVirtual arg = " << args[i] << "\n";

      if(args[i] == "print-virtual-callsite")
        opts->printCallSite = 1;
      else if(args[i] == "print-virtual-fn")
        opts->printVirtualFn = 1;
      else if(args[i] == "print-vf-member")
        opts->printVFMember = 1;
      else if(args[i] == "print-class-layout")
        opts->printClassLayout = 1;
      else if (args[i] == "help")
        PrintHelp(llvm::errs());
      else
        llvm::errs() << "Unsupported argument = " << args[i] << ", for plugin = print-virtual (PrintVirtual)\n";
    }
    return true;
  }

  void PrintHelp(llvm::raw_ostream& ros) {
    ros << "Clang Plugin PrintVirtual, enabled by Clang command line option: -plugin print-virtual\n";
    ros << "\t  -plugin-arg-print-virtual print-virtual-callsite \t Print all virtual callsites' information, including \n"
        << "\t                                                   \t callsite location, target vf name and location, and target class name and location\n";
    ros << "\t  -plugin-arg-print-virtual print-virtual-fn       \t Print all virtual functions' information, including \n"
        << "\t                                                   \t vf name and location, parent class name and location, and list of overridden virtual functions (name and location)\n";
    ros << "\t  -plugin-arg-print-virtual print-vf-member        \t Print all vf internal member access information, including \n"
        << "\t                                                   \t member type (data, data_ptr, fn_ptr), member access (read, write), member offset\n";
    ros << "\t  -plugin-arg-print-virtual print-class-layout     \t Print all classes' layout.\n";
    ros << "\t  -plugin-arg-print-virtual help                   \t Print this help message\n";
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 llvm::StringRef) override {
    return llvm::make_unique<PrintVirtualConsumer>(CI, opts, CI.getASTContext());
  }
};

}

static FrontendPluginRegistry::Add<PrintVirtualAction>
X("print-virtual", "print function names");
