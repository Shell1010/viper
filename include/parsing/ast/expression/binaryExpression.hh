#ifndef VIPER_AST_BINARY_HH
#define VIPER_AST_BINARY_HH
#include <parsing/ast/astNode.hh>
#include <lexing/token.hh>
#include <memory>

enum class BinaryOperator
{
    Addition, Subtraction,
    Multiplication, Division,

    Assignment
};

class BinaryExpression : public ASTNode
{
public:
    BinaryExpression(std::unique_ptr<ASTNode> lhs, Lexing::Token token, std::unique_ptr<ASTNode> rhs);

    void Print(std::ostream& stream, int indent) const override;

    Codegen::Value* Generate(Codegen::Module& module, Codegen::Builder& builder, bool isStatement = false) override;
private:
    std::unique_ptr<ASTNode> _lhs;
    BinaryOperator _operator;
    std::unique_ptr<ASTNode> _rhs;

    std::string OperatorToString() const;
};

#endif