#include "parser_base.hpp"
#include <iostream>

void Parser::parseStructDefinition(AST& ast) {
    // Skip 'struct' keyword
    advance();
    
    // Get struct name
    if (!hasTokens() || peekToken().type != TokenType::Identifier) {
        reportError("Expected struct name after 'struct' keyword");
    }
    
    std::string structName = peekToken().value.value_or("");
    advance();
    
    // Expect opening brace
    expectToken(TokenType::LeftBrace, "after struct name");
    
    std::vector<StructField> fields;
    
    // Parse fields
    while (hasTokens() && peekToken().type != TokenType::RightBrace) {
        // Parse field name
        if (peekToken().type != TokenType::Identifier) {
            reportError("Expected field name in struct definition");
        }
        
        std::string fieldName = peekToken().value.value_or("");
        advance();
        
        // Expect colon
        expectToken(TokenType::Colon, "after field name");
        
        // Parse field type
        SigType fieldType;
        std::optional<std::string> structTypeName;
        
        const auto& typeToken = peekToken();
        advance();
        
        switch (typeToken.type) {
            case TokenType::U8: fieldType = SigType::U8; break;
            case TokenType::U16: fieldType = SigType::U16; break;
            case TokenType::U32: fieldType = SigType::U32; break;
            case TokenType::U64: fieldType = SigType::U64; break;
            case TokenType::I8: fieldType = SigType::I8; break;
            case TokenType::I16: fieldType = SigType::I16; break;
            case TokenType::I32: fieldType = SigType::I32; break;
            case TokenType::I64: fieldType = SigType::I64; break;
            case TokenType::Identifier:
                // This could be another struct type
                fieldType = SigType::Struct;
                structTypeName = typeToken.value.value_or("");
                break;
            default:
                reportError("Invalid field type. Expected u8, u16, u32, u64, i8, i16, i32, i64, or struct name");
        }
        
        fields.push_back(StructField{fieldName, fieldType, structTypeName});
        
        // Expect comma or closing brace
        if (hasTokens() && peekToken().type == TokenType::Comma) {
            advance();
        } else if (hasTokens() && peekToken().type != TokenType::RightBrace) {
            reportError("Expected ',' between struct fields or '}' to end struct definition");
        }
    }
    
    // Expect closing brace
    expectToken(TokenType::RightBrace, "to close struct definition");
    
    // Create struct definition
    StructDefinition structDef{structName, fields};
    ast.push_back(structDef);
}

StructAccess Parser::parseStructAccess() {
    // Parse struct_name.field_name
    if (!hasTokens() || peekToken().type != TokenType::Identifier) {
        reportError("Expected struct variable name for member access");
    }
    
    std::string structName = peekToken().value.value_or("");
    advance();
    
    expectToken(TokenType::Dot, "for member access");
    
    if (!hasTokens() || peekToken().type != TokenType::Identifier) {
        reportError("Expected field name after '.'");
    }
    
    std::string fieldName = peekToken().value.value_or("");
    advance();
    
    return StructAccess{structName, fieldName};
}

StructInitialization Parser::parseStructInitialization() {
    // Parse StructType { field1: value1, field2: value2 }
    if (!hasTokens() || peekToken().type != TokenType::Identifier) {
        reportError("Expected struct type name for initialization");
    }
    
    std::string structType = peekToken().value.value_or("");
    advance();
    
    expectToken(TokenType::LeftBrace, "for struct initialization");
    
    std::vector<StructInitField> fields;
    
    while (hasTokens() && peekToken().type != TokenType::RightBrace) {
        // Parse field name
        if (peekToken().type != TokenType::Identifier) {
            reportError("Expected field name in struct initialization");
        }
        
        std::string fieldName = peekToken().value.value_or("");
        advance();
        
        expectToken(TokenType::Colon, "after field name in struct initialization");
        
        // Parse field value - for now, just support simple literals
        Expression value;
        const auto& valueToken = peekToken();
        advance();
        
        switch (valueToken.type) {
            case TokenType::IntegerLiteral:
                value = parseInteger(valueToken.value.value_or("0"));
                break;
            case TokenType::FloatLiteral:
                value = parseDouble(valueToken.value.value_or("0.0"));
                break;
            case TokenType::BooleanLiteral:
                value = (valueToken.value.value_or("false") == "true");
                break;
            case TokenType::String:
                value = valueToken.value.value_or("");
                break;
            case TokenType::HexLiteral:
                value = static_cast<int>(parseHexLiteral(valueToken.value.value_or("0x0")));
                break;
            default:
                reportError("Unsupported value type in struct initialization");
        }
        
        fields.push_back(StructInitField{fieldName, value});
        
        // Expect comma or closing brace
        if (hasTokens() && peekToken().type == TokenType::Comma) {
            advance();
        } else if (hasTokens() && peekToken().type != TokenType::RightBrace) {
            reportError("Expected ',' between struct fields or '}' to end struct initialization");
        }
    }
    
    expectToken(TokenType::RightBrace, "to close struct initialization");
    
    return StructInitialization{structType, fields};
}
