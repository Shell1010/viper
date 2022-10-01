#ifndef VIPER_AST_EXPRESSION_INTEGER_HH
#define VIPER_AST_EXPRESSION_INTEGER_HH
#include <parsing/ast/astNode.hh>

namespace Parsing
{
    class IntegerLiteral : public ASTNode
    {
    public:
        IntegerLiteral(long long value);

        void Print(std::ostream& stream, int indent) const override;

        long long GetValue() const;

        std::unique_ptr<SSA::Value> Emit() override;
    private:
        long long _value;
    };
}

#endif