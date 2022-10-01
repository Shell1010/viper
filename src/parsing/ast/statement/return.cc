#include <parsing/ast/statement/return.hh>

namespace Parsing
{
    ReturnStatement::ReturnStatement(std::unique_ptr<ASTNode> value)
        :_value(std::move(value))
    {
        _nodeType = ASTNodeType::ReturnStatement;
    }

    void ReturnStatement::Print(std::ostream& stream, int indent) const
    {
        stream << std::string(indent, ' ') << "<Return-Statement>";
        if(_value)
        {
            stream << ":\n" << std::string(indent, ' ') << "Value:\n";
            _value->Print(stream, indent + 2);
        }
    }

    std::unique_ptr<SSA::Value> ReturnStatement::Emit()
    {
        std::unique_ptr<SSA::Value> value = _value->Emit();

        return std::make_unique<SSA::Return>(value);
    }
}