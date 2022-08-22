#include <iostream>
#include <parsing/AST/expression/binaryExpression.hxx>
#include <parsing/AST/expression/variable.hxx>
#include <parsing/AST/expression/subscript.hxx>

namespace Viper
{
    namespace Parsing
    {
        BinaryExpression::BinaryExpression(std::unique_ptr<ASTNode> lhs, BinaryOperator op, std::unique_ptr<ASTNode> rhs)
            :_lhs(std::move(lhs)), _operator(op), _rhs(std::move(rhs))
        {
            _nodeType = ASTNodeType::BinaryExpression;
        }

        BinaryExpression::BinaryExpression(std::unique_ptr<ASTNode> lhs, Lexing::Token op, std::unique_ptr<ASTNode> rhs)
            :_lhs(std::move(lhs)), _rhs(std::move(rhs))
        {
            switch(op.getType())
            {
                case Lexing::TokenType::Plus:
                    _operator = BinaryOperator::Plus;
                    break;
                case Lexing::TokenType::Minus:
                    _operator = BinaryOperator::Minus;
                    break;
                case Lexing::TokenType::Star:
                    _operator = BinaryOperator::Multiply;
                    break;
                case Lexing::TokenType::Slash:
                    _operator = BinaryOperator::Divide;
                    break;
                case Lexing::TokenType::Equals:
                    _operator = BinaryOperator::Assignment;
                    break;
                case Lexing::TokenType::DoubleEquals:
                    _operator = BinaryOperator::Equals;
                    break;
                case Lexing::TokenType::BangEquals:
                    _operator = BinaryOperator::NotEquals;
                    break;
                case Lexing::TokenType::DoubleAmpersand:
                    _operator = BinaryOperator::And;
                    break;
                case Lexing::TokenType::DoublePipe:
                    _operator = BinaryOperator::Or;
                    break;
                case Lexing::TokenType::LessThan:
                    _operator = BinaryOperator::LessThan;
                    break;
                case Lexing::TokenType::GreaterThan:
                    _operator = BinaryOperator::GreaterThan;
                    break;
                case Lexing::TokenType::LessEquals:
                    _operator = BinaryOperator::LessEquals;
                    break;
                case Lexing::TokenType::GreaterEquals:
                    _operator = BinaryOperator::GreaterEquals;
                    break;
                default: // This should never be reached
                    break;
            }
        }

        void BinaryExpression::Print(std::ostream& stream) const
        {
            stream << "<Binary-Expression>:\nLhs: ";
            _lhs->Print(stream);
            stream << "\nOperator: " << OperatorToString() << "\nRhs: ";
            _rhs->Print(stream);
        }

        std::string BinaryExpression::OperatorToString() const
        {
            switch(_operator)
            {
                case BinaryOperator::Plus:
                    return "Addition";
                case BinaryOperator::Minus:
                    return "Subtraction";
                case BinaryOperator::Multiply:
                    return "Multiplication";
                case BinaryOperator::Divide:
                    return "Division";
                case BinaryOperator::Assignment:
                    return "Assignment";
                case BinaryOperator::Equals:
                    return "Equals";
                case BinaryOperator::NotEquals:
                    return "NotEquals";
                case BinaryOperator::And:
                    return "And";
                case BinaryOperator::Or:
                    return "Or";
                case BinaryOperator::LessThan:
                    return "LessThan";
                case BinaryOperator::GreaterThan:
                    return "GreaterThan";
                case BinaryOperator::LessEquals:
                    return "LessEquals";
                case BinaryOperator::GreaterEquals:
                    return "GreaterEquals";
            }
        }

        llvm::Value* BinaryExpression::Generate(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::shared_ptr<Environment> scope, std::vector<CodegenFlag>)
        {
            if(_operator == BinaryOperator::Assignment)
            {
                if(_lhs->GetNodeType() == ASTNodeType::Variable)
                {
                    Variable* left = static_cast<Variable*>(_lhs.get());
                    llvm::Value* value = _rhs->Generate(context, builder, module, scope);

                    llvm::AllocaInst* alloca = FindNamedValue(left->GetName(), scope);

                    if(value->getType() != alloca->getAllocatedType())
                        value = Type::Convert(value, alloca->getAllocatedType(), builder);

                    return builder.CreateStore(value, alloca);
                }
                else if(_lhs->GetNodeType() == ASTNodeType::SubscriptExpression)
                {
                    //SubscriptExpression* left = static_cast<SubscriptExpression*>(_lhs.get());

                    llvm::Value* rightCodegen = _rhs->Generate(context, builder, module, scope);
                    llvm::Value* leftCodegen = _lhs->Generate(context, builder, module, scope, { CodegenFlag::NoLoad });

                    if(leftCodegen->getType()->getNonOpaquePointerElementType() != rightCodegen->getType())
                        rightCodegen = Type::Convert(rightCodegen, leftCodegen->getType()->getNonOpaquePointerElementType(), builder);

                    return builder.CreateStore(rightCodegen, leftCodegen);
                }
            }

            llvm::Value* left  = _lhs->Generate(context, builder, module, scope);
            llvm::Value* right = _rhs->Generate(context, builder, module, scope);

            if(left->getType() != right->getType())
                left = Type::Convert(left, right->getType(), builder);

            switch(_operator)
            {
                case BinaryOperator::Plus:
                    return builder.CreateAdd(left, right, "add");
                case BinaryOperator::Minus:
                    return builder.CreateSub(left, right, "sub");
                case BinaryOperator::Multiply:
                    return builder.CreateMul(left, right, "mul");
                case BinaryOperator::Divide:
                    return builder.CreateSDiv(left, right, "div");
                
                case BinaryOperator::Equals:
                    return builder.CreateICmpEQ(left, right, "eq");
                case BinaryOperator::NotEquals:
                    return builder.CreateICmpNE(left, right, "ne");
                
                case BinaryOperator::And:
                    return builder.CreateLogicalAnd(left, right, "and");
                case BinaryOperator::Or:
                    return builder.CreateLogicalOr(left, right, "or");
                
                case BinaryOperator::LessThan:
                    return builder.CreateICmpSLT(left, right, "lt");
                case BinaryOperator::GreaterThan:
                    return builder.CreateICmpSGT(left, right, "gt");
                case BinaryOperator::LessEquals:
                    return builder.CreateICmpSLE(left, right, "le");
                case BinaryOperator::GreaterEquals:
                    return builder.CreateICmpSGE(left, right, "ge");
                default: // Should never be reached
                    return nullptr;
            }
        }
    }
}