#include <parsing/parser.hh>
#include <symbol/symbols.hh>
#include <type/types.hh>
#include <environment.hh>
#include <diagnostics.hh>

namespace Parsing
{
    Parser::Parser(const std::vector<Lexing::Token>& tokens, const std::string& text)
        :_text(text), _tokens(tokens), _position(0), _currentReturnType(nullptr)
    {
    }

    Lexing::Token Parser::Current() const
    {
        return _tokens[_position];
    }

    Lexing::Token Parser::Consume()
    {
        return _tokens[_position++];
    }

    Lexing::Token Parser::Peek(const int offset) const
    {
        return _tokens[_position + offset];
    }

    int Parser::GetBinOpPrecedence(Lexing::TokenType type)
    {
        switch(type)
        {
            case Lexing::TokenType::Star:
            case Lexing::TokenType::Slash:
                return 40;
            
            case Lexing::TokenType::Plus:
            case Lexing::TokenType::Minus:
                return 35;

            case Lexing::TokenType::Equals:
                return 10;
            
            default:
                return 0;
        }
    }

    void Parser::ExpectToken(Lexing::TokenType tokenType)
    {
        if(Current().GetType() != tokenType)
        {
            Lexing::Token temp(tokenType, "", 0, 0, 0, 0);

            ParserError("Expected '" + temp.TypeAsString() + "', found " + Current().GetText());
        }
    }

    void Parser::ParserError(std::string message)
    {
        unsigned int start = Current().GetStart();
        while(_text[start] != '\n')
            start--;
        unsigned int end = Current().GetEnd();
        while(_text[end] != '\n')
            end++;
        Diagnostics::CompilerError(Current().GetLine(), Current().GetCol(),
        message, &_text[Current().GetStart()], &_text[Current().GetEnd()],
                &_text[start], &_text[end]);
    }


    std::vector<std::unique_ptr<ASTNode>> Parser::Parse()
    {
        std::vector<std::unique_ptr<ASTNode>> result;
        while(_position < _tokens.size())
        {
            int savePos = _position;
            std::unique_ptr<ASTNode> expr = ParseExpression();
            ExpectToken(Lexing::TokenType::Semicolon);
            Consume();

            if(expr->GetNodeType() == ASTNodeType::Function)
                result.push_back(std::move(expr));
            else
            {
                _position = savePos;
                ParserError("Expected top-level expression");
            }
        }
        return result;
    }

    std::shared_ptr<Type> Parser::ParseType()
    {
        ExpectToken(Lexing::TokenType::Type);
        return types.at(Consume().GetText());
    }
    
    std::unique_ptr<ASTNode> Parser::ParseExpression(int precedence)
    {
        std::unique_ptr<ASTNode> lhs = ParsePrimary();

        while(true)
        {
            int binOpPrecedence = GetBinOpPrecedence(Current().GetType());
            if(binOpPrecedence < precedence)
                break;

            Lexing::Token operatorToken = Consume();
            std::unique_ptr<ASTNode> rhs = ParseExpression(binOpPrecedence);
            lhs = std::make_unique<BinaryExpression>(std::move(lhs), operatorToken, std::move(rhs));
        }

        return lhs;
    }

    std::unique_ptr<ASTNode> Parser::ParsePrimary()
    {
        switch(Current().GetType())
        {
            case Lexing::TokenType::Let:
                return ParseVariableDeclaration();
            case Lexing::TokenType::Identifier:
                return ParseIdentifier();
            case Lexing::TokenType::Return:
                return ParseReturnStatement();
            case Lexing::TokenType::Integer:
                return ParseIntegerLiteral();
            case Lexing::TokenType::LeftParen:
                return ParseParenthesizedExpression();
            case Lexing::TokenType::LeftBracket:
                return ParseCompoundExpression();
            default:
                ParserError("Expected primary expression, found '" + Current().GetText() + "'");
        }
    }

    std::unique_ptr<ASTNode> Parser::ParseIdentifier()
    {
        switch(Peek(1).GetType())
        {
            case Lexing::TokenType::LeftParen:
                return ParseCallExpression();
            default:
                return ParseVariable();
        }
    }

    std::unique_ptr<ASTNode> Parser::ParseVariableDeclaration()
    {
        Consume();

        std::shared_ptr<Type> type = ParseType();

        ExpectToken(Lexing::TokenType::Identifier);
        std::string name = Consume().GetText();

        bool isFunction = false;
        if(Current().GetType() == Lexing::TokenType::LeftParen)
        {
            Consume();
            isFunction = true;
            // TODO: Parse args
            ExpectToken(Lexing::TokenType::RightParen);
            Consume();
            _currentReturnType = type;
        }

        if(!isFunction)
            varSymbols.push_back(new VarSymbol(name, type));

        if(Current().GetType() != Lexing::TokenType::Equals)
            return std::make_unique<VariableDeclaration>(name, type, nullptr, isFunction);

        Consume();
        
        std::unique_ptr<ASTNode> initVal = ParseExpression();
        if(isFunction)
        {
            if(initVal->GetNodeType() != ASTNodeType::CompoundStatement && initVal->GetNodeType() != ASTNodeType::ReturnStatement)
                initVal = std::make_unique<ReturnStatement>(std::move(initVal), _currentReturnType);
        }
        
        return std::make_unique<VariableDeclaration>(name, type, std::move(initVal), isFunction);
    }

    std::unique_ptr<ASTNode> Parser::ParseVariable()
    {
        std::string name = Consume().GetText();
        VarSymbol* symbol = FindSymbol(name);
        if(!symbol)
        {
            --_position;
            ParserError("Undeclared identifier: `" + name + "'.");
        }
        return std::make_unique<Variable>(name, symbol->GetType());
    }

    std::unique_ptr<ASTNode> Parser::ParseCallExpression()
    {
        std::string callee = Consume().GetText();
        ExpectToken(Lexing::TokenType::LeftParen);
        Consume();
        // TODO: Parse args
        ExpectToken(Lexing::TokenType::RightParen);
        Consume();

        return std::make_unique<CallExpr>(callee);
    }

    std::unique_ptr<ASTNode> Parser::ParseIntegerLiteral()
    {
        long long value = std::stoll(Consume().GetText());

        return std::make_unique<IntegerLiteral>(value);
    }

    std::unique_ptr<ASTNode> Parser::ParseReturnStatement()
    {
        Consume();

        if(Current().GetType() == Lexing::TokenType::Semicolon)
            return std::make_unique<ReturnStatement>(nullptr, _currentReturnType);

        return std::make_unique<ReturnStatement>(ParseExpression(), _currentReturnType);
    }

    std::unique_ptr<ASTNode> Parser::ParseParenthesizedExpression()
    {
        Consume();

        std::unique_ptr<ASTNode> expr = ParseExpression();

        ExpectToken(Lexing::TokenType::RightParen);
        Consume();

        return expr;
    }

    std::unique_ptr<ASTNode> Parser::ParseCompoundExpression()
    {
        Consume();

        std::vector<std::unique_ptr<ASTNode>> exprs;

        while(Current().GetType() != Lexing::TokenType::RightBracket)
        {
            exprs.push_back(ParseExpression());
            ExpectToken(Lexing::TokenType::Semicolon);
            Consume();
        }
        Consume();

        _tokens.insert(_tokens.begin() + _position, Lexing::Token(Lexing::TokenType::Semicolon, "", 0, 0, 0, 0));

        return std::make_unique<CompoundStatement>(exprs);
    }
}