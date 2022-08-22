#ifndef VIPER_AST_VARIABLEDECL_HXX
#define VIPER_AST_VARIABLEDECL_HXX
#include <parsing/AST/astNode.hxx>

namespace Viper
{
    namespace Parsing
    {
        class VariableDeclaration : public ASTNode
        {
        public:
            VariableDeclaration(std::shared_ptr<Type> type, std::string name, std::unique_ptr<ASTNode> value);

            void Print(std::ostream& stream) const override;

            llvm::Value* Generate(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::shared_ptr<Environment> scope, std::vector<CodegenFlag> flags = {}) override;
        private:
            std::string _name;
            std::unique_ptr<ASTNode> _value;
        };
    }
}

#endif