// Interpretter.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <list>
#include <tuple>
#include <vector>
#include <stdlib.h>
#include <unordered_map>
#include <functional>
#include <memory>
#include <math.h>

//TODO: Implement variable/function hashmap
class Parser;
class Lexer;
class Operation;
class BaseToken;

std::unordered_map <std::string, int> PriorityMap
{
    {"(",1},
    {"function",2},
    {"^",3},
    {"*",4},
    {"/",4},
    {"+",5},
    {"-",5},
    {"==",6},
    {"and",7},
    {"or",8},
};

std::unordered_map <std::string, std::function<std::shared_ptr<Operation> (Parser*, std::shared_ptr<BaseToken>)>> ProcessMap;

bool TestIfDigit(char &Digit) {
    char Numbers[] = { '0','1','2','3','4','5','6','7','8','9'};

    for (int i = 0; i < 11; i++) {
        if (Numbers[i] == Digit) {
            return true;
        }
    }
    return false;
};

struct BaseToken {
    std::string Type;
    std::string SubType;
};

struct Operation {
    std::string Type;
    std::vector<std::string> MemberTypes;
    std::vector<std::shared_ptr<BaseToken>> Arguments;
    Operation(std::string OperationInp, std::vector<std::shared_ptr<BaseToken>> Args) {
        this->Arguments = Args;
        this->Type = OperationInp;
        
        int TotalArgs = Args.size();
        for (int i = 0; i < TotalArgs; i++) {
            MemberTypes.push_back((*Args[i]).Type);
        }
    }
};

template <typename RandType>
struct Token: public BaseToken {
    RandType Value;
    Token(std::string TypeInp, RandType ValueInp, std::string SubType = "") {
        this->Value = ValueInp;
        this->Type = TypeInp;
        if (SubType == "") {
            this->SubType = TypeInp;
        }
        else {
            this->SubType = SubType;
        }
    }
};

std::vector<Token<std::string>*> Tokens = { new Token <std::string>("==","=="),new Token <std::string>(",",","),new Token <std::string>("=","="),new Token <std::string>("*","*"),new Token <std::string>("-","-"),new Token <std::string>("+","+"),new Token <std::string>("/","/"),new Token <std::string>("(","("),new Token <std::string>(")",")"),new Token <std::string>("^","^") };
std::vector<Token<std::string>*> KeywordTokens = { new Token <std::string>("and","and"),new Token <std::string>("or","or"),new Token <std::string>("true","true"), new Token <std::string>("false","false"), new Token <std::string>("then","then"),new Token <std::string>("function","print"),new Token <std::string>("function","log"),new Token <std::string>("function","ln"),new Token <std::string>("if","if"),new Token <std::string>("end","end"),new Token <std::string>("while","while"),new Token <std::string>("nil","nil") };

class Lexer {
    int Position = 0;
    std::string Text;
    char CurrentCharacter;
public:
    int CurrentLine = 0;

    Lexer(std::string TextInp) {
        this->Text = TextInp;
        this->CurrentCharacter = this->Text[Position];
    };
    void MultiAdvance(const int Iterations) {
        for (int i = 0; i < Iterations; i++) {
            this->Advance();
        }
    }
    void Advance() {
        this->Position++;
        if (this->Position-1 >= this->Text.length()) {
            this->CurrentCharacter = '\0';
        }
        else {
            this->CurrentCharacter = this->Text[Position];
        }
    }
    void SkipWhitespace() {
        char Whitespace[] = {' ', '\t','\n' };
        for (int i = 0; i < 3; i++)
        {
            if (this->CurrentCharacter == Whitespace[i])
            {
                if (this->CurrentCharacter == '\n') {
                    this->CurrentLine++;
                }
                this->Advance();
                SkipWhitespace();
                break;
            }
        }
    };
    //Future goal is to optimize it for any/all types
    std::shared_ptr<BaseToken> GetNumber() {
        std::string Result = "";
        bool FloatingPoint = false;
        
        while (TestIfDigit(this->CurrentCharacter) || this->CurrentCharacter == '.') {
            Result += this->CurrentCharacter;
            if (this->CurrentCharacter == '.') {
                FloatingPoint = true;
            }
            this->Advance();
        }
        return std::shared_ptr<Token<double>>(new Token<double>("number", std::stod(Result)));
    }

    std::shared_ptr<BaseToken> GetString(char Quotation) {
        std::string Result = "";
        this->Advance();

        while (this->CurrentCharacter != Quotation) {
            Result += this->CurrentCharacter;
            this->Advance();
        }
        this->Advance();
        return std::shared_ptr<BaseToken> (new Token<std::string>("string", Result));
    }
    char TestForQuotations(const char character) {
        if (character == '\'') {
            return '\'';
        }
        if (character == '\"') {
            return '\"';
        }
        return '\0';
    }

    //ADD HASHMAPS FOR SPEED BOOST IN FUTURE!
    bool NonLetterCheck(const char Character) {
        std::string AllLetters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890";
        for (int i = 0; i < AllLetters.size(); i++) {
            if (AllLetters[i] == Character) {
                return false;
            }
        }
return true;
    }
    bool TokenCompare(Token<std::string>* TokenInp) {
        std::string Result;

        for (int i = 0; i < (*TokenInp).Value.size(); i++) {
            if (this->Position + i + 1 > this->Text.size() || (*TokenInp).Value[i] != this->Text[this->Position + i]) {
                return false;
            }
        }
        return true;
    }
    void RaiseException(std::string Message = "Illegal Syntax used") {
        //throw std::invalid_argument(Message);
    }

    std::shared_ptr<BaseToken> NextToken() {
        while (this->CurrentCharacter != '\0') {
            this->SkipWhitespace();

            if (TestIfDigit(this->CurrentCharacter)) {
                return GetNumber();
            }


            if (TestForQuotations(this->CurrentCharacter) != '\0') {
                return GetString(TestForQuotations(this->CurrentCharacter));
            }

            for (int i = 0; i < Tokens.size(); i++) {
                if (TokenCompare(Tokens[i])) {
                    BaseToken* NewToken = static_cast<BaseToken*>(new Token<std::string>(Tokens[i]->Type, Tokens[i]->Value, Tokens[i]->SubType));
                    MultiAdvance(Tokens[i]->Value.size());
                    return std::shared_ptr<BaseToken>(NewToken);
                }
            }

            //Implement keywords

            for (int i = 0; i < KeywordTokens.size(); i++) {
                if (TokenCompare(KeywordTokens[i]) && this->Position + (*KeywordTokens[i]).Value.size() < this->Text.size() && NonLetterCheck(this->Text[this->Position + (*KeywordTokens[i]).Value.size()])) {
                    BaseToken* NewToken = static_cast<BaseToken*>(new Token<std::string>(KeywordTokens[i]->Type, KeywordTokens[i]->Value, KeywordTokens[i]->SubType));
                    MultiAdvance(KeywordTokens[i]->Value.size());
                    return std::shared_ptr<BaseToken>(NewToken);
                }
            }

            std::string ErrorMessage = static_cast<std::string>("Error on line ");
            ErrorMessage += (std::to_string(this->CurrentLine) + static_cast<std::string>(": Illegal symbol \'") + this->CurrentCharacter + static_cast<std::string>("\' found."));
            this->RaiseException(ErrorMessage);
        }
        BaseToken* NewToken = new Token<char>("EOF", '\0');
        return std::shared_ptr<BaseToken>(NewToken);
    }
};



//Interpret operations
std::shared_ptr<BaseToken> Interpret(std::shared_ptr<Operation> CurrentOperation) {

    //Low level operation/Tokens
    std::vector<std::string> BaseTokens = { "number","string","variable","nil","true","false"};

    for (int i = 0; i < BaseTokens.size(); i++) {
        if ((*CurrentOperation).Type == BaseTokens[i]) {
            std::shared_ptr<BaseToken> NewToken = CurrentOperation->Arguments[0];
            return NewToken;
        }
    }

    //Operation Implementation

    //Type optimization later
    if ((*CurrentOperation).Type == "*") {
        double NewValue = (static_cast<Token<double>*>(CurrentOperation->Arguments[0].get())->Value * static_cast<Token<double>*>(CurrentOperation->Arguments[1].get())->Value);
        BaseToken* NewToken = new Token<double>("number", NewValue, (*CurrentOperation).Type);
        return std::shared_ptr<BaseToken>(NewToken);
    }
    if ((*CurrentOperation).Type == "/") {
        double NewValue = (static_cast<Token<double>*>(CurrentOperation->Arguments[0].get())->Value / static_cast<Token<double>*>(CurrentOperation->Arguments[1].get())->Value);
        BaseToken* NewToken = new Token<double>("number", NewValue, (*CurrentOperation).Type);
        return std::shared_ptr<BaseToken>(NewToken);
    }
    if ((*CurrentOperation).Type == "+") {
        double NewValue = (static_cast<Token<double>*>(CurrentOperation->Arguments[0].get())->Value + static_cast<Token<double>*>(CurrentOperation->Arguments[1].get())->Value);
        BaseToken* NewToken = new Token<double>("number", NewValue, (*CurrentOperation).Type);
        return std::shared_ptr<BaseToken>(NewToken);
    }
    if ((*CurrentOperation).Type == "-") {
        double NewValue = (static_cast<Token<double>*>(CurrentOperation->Arguments[0].get())->Value - static_cast<Token<double>*>(CurrentOperation->Arguments[1].get())->Value);
        BaseToken* NewToken = new Token<double>("number", NewValue, (*CurrentOperation).Type);
        return std::shared_ptr<BaseToken>(NewToken);
    }
    if ((*CurrentOperation).Type == "^") {
        double NewValue = static_cast<double>(std::pow(static_cast<Token<double>*>(CurrentOperation->Arguments[0].get())->Value, static_cast<Token<double>*>(CurrentOperation->Arguments[1].get())->Value));
        BaseToken* NewToken = new Token<double>("number", NewValue, (*CurrentOperation).Type);
        return std::shared_ptr<BaseToken>(NewToken);
    }
    if ((*CurrentOperation).Type == "and") {
        if (CurrentOperation->Arguments[0].get()->Type == "false" || CurrentOperation->Arguments[0].get()->Type == "nil" || (CurrentOperation->Arguments[0].get()->Type == "number" && static_cast<Token<double>*>(CurrentOperation->Arguments[0].get()) == 0)) {
            return CurrentOperation->Arguments[0];
        }
        else {
            return CurrentOperation->Arguments[1];
        }
    }
    if ((*CurrentOperation).Type == "or") {
        if ((CurrentOperation->Arguments[0].get()->Type == "false" || CurrentOperation->Arguments[0].get()->Type == "nil" || (CurrentOperation->Arguments[0].get()->Type == "number" && static_cast<Token<double>*>(CurrentOperation->Arguments[0].get()) == 0)) && (CurrentOperation->Arguments[1].get()->Type == "false" || CurrentOperation->Arguments[1].get()->Type == "nil" || (CurrentOperation->Arguments[1].get()->Type == "number" && static_cast<Token<double>*>(CurrentOperation->Arguments[1].get()) == 0))) {
            return CurrentOperation->Arguments[0];
        }
        else {
            if (CurrentOperation->Arguments[0].get()->Type == "false" || CurrentOperation->Arguments[0].get()->Type == "nil" || (CurrentOperation->Arguments[0].get()->Type == "number" && static_cast<Token<double>*>(CurrentOperation->Arguments[0].get()) == 0)) {
                return CurrentOperation->Arguments[1];
            }
            else {
                return CurrentOperation->Arguments[0];
            }
        }
    }
    if ((*CurrentOperation).Type == "==") {
        if (CurrentOperation->Arguments[0].get()->Type != CurrentOperation->Arguments[1].get()->Type) {
            BaseToken* NewToken = new Token<std::string>("false", "false");
            return std::shared_ptr<BaseToken>(NewToken);
        }
        else {
            if (CurrentOperation->Arguments[0].get()->Type == "true" || CurrentOperation->Arguments[1].get()->Type == "false" || CurrentOperation->Arguments[1].get()->Type == "nil") {
                BaseToken* NewToken = new Token<std::string>("true", "true");
                return std::shared_ptr<BaseToken>(NewToken);
            }
            if (CurrentOperation->Arguments[0].get()->Type == "number" && (static_cast<Token<double>*>(CurrentOperation->Arguments[0].get())->Value == static_cast<Token<double>*>(CurrentOperation->Arguments[1].get())->Value)) {
                BaseToken* NewToken = new Token<std::string>("true", "true");
                return std::shared_ptr<BaseToken>(NewToken);
            }
            if (CurrentOperation->Arguments[0].get()->Type == "string" && (static_cast<Token<std::string>*>(CurrentOperation->Arguments[0].get())->Value == static_cast<Token<std::string>*>(CurrentOperation->Arguments[1].get())->Value)) {
                BaseToken* NewToken = new Token<std::string>("true", "true");
                return std::shared_ptr<BaseToken>(NewToken);
            }
        }
    }
    if ((*CurrentOperation).Type == "or") {
        double NewValue = static_cast<double>(std::pow(static_cast<Token<double>*>(CurrentOperation->Arguments[0].get())->Value, static_cast<Token<double>*>(CurrentOperation->Arguments[1].get())->Value));
        BaseToken* NewToken = new Token<double>("number", NewValue, (*CurrentOperation).Type);
        return std::shared_ptr<BaseToken>(NewToken);
    }
};

//Current implementation of the AST


//SyntaxTree AST();


class Parser {
    Lexer CurrentLexer;
public:
    std::shared_ptr<BaseToken> CurrentToken;
    Parser(std::string Text) : CurrentLexer(Text)  {
        this->CurrentToken = this->CurrentLexer.NextToken();
    }
    void ParseToken(std::string TokenType) {
        if ((*(this->CurrentToken)).Type == TokenType) {
            this->CurrentToken = this->CurrentLexer.NextToken();
        }
        else {
            std::string ErrorMessage = static_cast<std::string>("Error on line ");
            ErrorMessage += (std::to_string(this->CurrentLexer.CurrentLine) + static_cast<std::string>(": Unexpected token \'") + (*(this->CurrentToken)).Type + static_cast<std::string>("\' found when \'") + TokenType + static_cast<std::string>("\' expected."));
            this->CurrentLexer.RaiseException(ErrorMessage);
        }
    } 
    bool ParseIfExists(std::string TokenType) {
        if ((*(this->CurrentToken)).Type == TokenType) {
            this->ParseToken(TokenType);
            return true;
        }
        return false;
    }
    //USE THE TABLE METHOD TO LOWER HOW MUCH CODE IS HERE!!!!!
    std::shared_ptr<Operation> ReturnBaseTokens() {
        if ((*(this->CurrentToken)).Type == "number") {
            Operation* NewOperation = new Operation("number", std::vector<std::shared_ptr<BaseToken>>{(this->CurrentToken)});
            this->ParseToken("number");
            return std::shared_ptr<Operation>(NewOperation);
        }
        if ((*(this->CurrentToken)).Type == "string") {
            Operation* NewOperation = new Operation("string", std::vector<std::shared_ptr<BaseToken>>{(this->CurrentToken)});
            this->ParseToken("string");
            return std::shared_ptr<Operation>(NewOperation);
        }
        if ((*(this->CurrentToken)).Type == "false") {
            Operation* NewOperation = new Operation("false", std::vector<std::shared_ptr<BaseToken>>{(this->CurrentToken)});
            this->ParseToken("false");
            return std::shared_ptr<Operation>(NewOperation);
        }
        if ((*(this->CurrentToken)).Type == "true") {
            Operation* NewOperation = new Operation("true", std::vector<std::shared_ptr<BaseToken>>{(this->CurrentToken)});
            this->ParseToken("true");
            return std::shared_ptr<Operation>(NewOperation);
        }
        if ((*(this->CurrentToken)).Type == "nil") {
            Operation* NewOperation = new Operation("mil", std::vector<std::shared_ptr<BaseToken>>{(this->CurrentToken)});
            this->ParseToken("nil");
            return std::shared_ptr<Operation>(NewOperation);
        }
        if ((*(this->CurrentToken)).Type == "variable") { //WILL BE REPLACED, VARIABLES ARE A SUBTYPE
            Operation* NewOperation = new Operation("variable", std::vector<std::shared_ptr<BaseToken>>{(this->CurrentToken)});
            this->ParseToken("variable");
            return std::shared_ptr<Operation>(NewOperation);
        }
        return std::shared_ptr<Operation>(nullptr);
    }
    std::shared_ptr<Operation> SyntaxTraverse(std::shared_ptr<Operation> NodeInp = std::shared_ptr<Operation>(nullptr),bool Override = false) {
        std::shared_ptr<Operation> FirstNode = NodeInp;

        if (FirstNode.get() == nullptr) {
            FirstNode = this->ReturnBaseTokens();
        }
        if (FirstNode.get() == nullptr) {
            FirstNode = ProcessMap[(*(this->CurrentToken)).Type](this, std::shared_ptr<BaseToken>(nullptr));
        }
        if ((this->CurrentToken)->Type == "EOF" || (PriorityMap[(*(this->CurrentToken)).Type] == 0) ||(Override && PriorityMap[FirstNode->Type] == 0)) {
            if (FirstNode.get() == nullptr) {
                return std::shared_ptr<Operation>(new Operation("nil", std::vector<std::shared_ptr<BaseToken>>{ std::shared_ptr<Token<std::string>>(new Token <std::string>("nil", "nil")) }));
            }
            return FirstNode;
        }
        if (NodeInp.get() == nullptr || (PriorityMap[NodeInp->Type] <= PriorityMap[(*(this->CurrentToken)).Type])) {
            std::shared_ptr<BaseToken> NewFirstNode = Interpret(FirstNode);
            return ProcessMap[(*(this->CurrentToken)).Type](this, (NewFirstNode));
        }
        else {
            Operation* NewNode = new Operation(NodeInp->Type, std::vector<std::shared_ptr<BaseToken>>{(FirstNode->Arguments[0]), Interpret(ProcessMap[(*(this->CurrentToken)).Type](this, (FirstNode->Arguments[1])))});
            return (this->SyntaxTraverse(std::shared_ptr<Operation>(NewNode)));
        }
    }
};

std::shared_ptr<Operation> ConvertToBase(std::shared_ptr<Operation> NewOperation) {
    std::shared_ptr<BaseToken> NewToken = Interpret(NewOperation);
    return std::shared_ptr<Operation>(new Operation(NewToken->Type, std::vector<std::shared_ptr<BaseToken>>{ NewToken }));
};

//Lowest Level Token Interpretations
std::shared_ptr<Operation> LParen(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = std::shared_ptr<BaseToken>(nullptr)) {
    CurrentParser->ParseToken("(");
    std::shared_ptr<Operation> CurrentOperation = CurrentParser->SyntaxTraverse();
    CurrentParser->ParseToken(")");
    return CurrentParser->SyntaxTraverse(ConvertToBase(CurrentOperation)); // Take prior stuff
};

std::shared_ptr<Operation> Mult(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = std::shared_ptr<BaseToken>(nullptr)) { //ADD ERROR CHECKING
    std::shared_ptr<BaseToken> FirstNode = (Node);
    CurrentParser->ParseToken("*");
    std::shared_ptr<Operation> SecondNodeOP = CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(nullptr),true);
    std::shared_ptr<BaseToken> SecondNode = Interpret(SecondNodeOP);
    return CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(new Operation("*", std::vector<std::shared_ptr<BaseToken>>{(FirstNode), (SecondNode)})));
};

std::shared_ptr<Operation> Plus(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = std::shared_ptr<BaseToken>(nullptr)) { //ADD ERROR CHECKING
    std::shared_ptr<BaseToken> FirstNode = (Node);
    CurrentParser->ParseToken("+");
    std::shared_ptr<Operation> SecondNodeOP = CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(nullptr), true);
    std::shared_ptr<BaseToken> SecondNode = Interpret(SecondNodeOP);
    return CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(new Operation("+", std::vector<std::shared_ptr<BaseToken>>{(FirstNode), (SecondNode)})));
};

std::shared_ptr<Operation> Minus(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = std::shared_ptr<BaseToken>(nullptr)) { //ADD ERROR CHECKING
    std::shared_ptr<BaseToken> FirstNode = (Node);
    CurrentParser->ParseToken("-");
    std::shared_ptr<Operation> SecondNodeOP = CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(nullptr), true);
    std::shared_ptr<BaseToken> SecondNode = Interpret(SecondNodeOP);
    return CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(new Operation("-", std::vector<std::shared_ptr<BaseToken>>{(FirstNode), (SecondNode)})));
};

std::shared_ptr<Operation> Divide(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = nullptr) { //ADD ERROR CHECKING
    std::shared_ptr<BaseToken> FirstNode = (Node);
    CurrentParser->ParseToken("/");
    std::shared_ptr<Operation> SecondNodeOP = CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(nullptr), true);
    std::shared_ptr<BaseToken> SecondNode = Interpret(SecondNodeOP);
    return CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(new Operation("/", std::vector<std::shared_ptr<BaseToken>>{(FirstNode), (SecondNode)})));
};

std::shared_ptr<Operation> RaisePower(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = nullptr) { //ADD ERROR CHECKING
    std::shared_ptr<BaseToken> FirstNode = (Node);
    CurrentParser->ParseToken("^");
    std::shared_ptr<Operation> SecondNodeOP = CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(nullptr), true);
    std::shared_ptr<BaseToken> SecondNode = Interpret(SecondNodeOP);
    return CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(new Operation("^", std::vector<std::shared_ptr<BaseToken>>{(FirstNode), (SecondNode)})));
};

std::shared_ptr<Operation> PerformAnd(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = nullptr) { //ADD ERROR CHECKING
    std::shared_ptr<BaseToken> FirstNode = (Node);
    CurrentParser->ParseToken("and");
    std::shared_ptr<Operation> SecondNodeOP = CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(nullptr), true);
    std::shared_ptr<BaseToken> SecondNode = Interpret(SecondNodeOP);
    return CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(new Operation("and", std::vector<std::shared_ptr<BaseToken>>{(FirstNode), (SecondNode)})));
};

std::shared_ptr<Operation> PerformOr(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = nullptr) { //ADD ERROR CHECKING
    std::shared_ptr<BaseToken> FirstNode = (Node);
    CurrentParser->ParseToken("or");
    std::shared_ptr<Operation> SecondNodeOP = CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(nullptr), true);
    std::shared_ptr<BaseToken> SecondNode = Interpret(SecondNodeOP);
    return CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(new Operation("or", std::vector<std::shared_ptr<BaseToken>>{(FirstNode), (SecondNode)})));
};

std::shared_ptr<Operation> PerformEquivalent(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = nullptr) { //ADD ERROR CHECKING
    std::shared_ptr<BaseToken> FirstNode = (Node);
    CurrentParser->ParseToken("==");
    std::shared_ptr<Operation> SecondNodeOP = CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(nullptr), true);
    std::shared_ptr<BaseToken> SecondNode = Interpret(SecondNodeOP);
    return CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(new Operation("==", std::vector<std::shared_ptr<BaseToken>>{(FirstNode), (SecondNode)})));
};

//BUILT IN FUNCTIONS
//USES TUPLE OPERATION!

std::shared_ptr<BaseToken> CustomLn(std::shared_ptr<Operation> InputTuple) { //ADD ERROR CHECKING
    double Input = static_cast<Token<double>*>(InputTuple->Arguments[0].get())->Value;
    BaseToken* NewToken = new Token<double>("number", std::log(Input));
    return std::shared_ptr<BaseToken>(NewToken);
};

std::shared_ptr<BaseToken> CustomLog(std::shared_ptr<Operation> InputTuple) { //ADD ERROR CHECKING
    double Input = static_cast<Token<double>*>(InputTuple->Arguments[0].get())->Value;
    BaseToken* NewToken = new Token<double>("number", std::log(Input)/std::log(10));
    return std::shared_ptr<BaseToken>(NewToken);
};

std::shared_ptr<BaseToken> CustomPrint(std::shared_ptr<Operation> InputTuple) { //ADD ERROR CHECKING

    //Change the output stream in the future!
    for (std::shared_ptr<BaseToken> v : InputTuple->Arguments) {
        if (v->Type == "number") {
            Token<double> Result = *(static_cast<Token<double>*>(v.get()));
            std::cout << Result.Value;
        }
        if (v->Type == "string" || v->Type == "true" || v->Type == "false" || v->Type == "nil") {
            Token<std::string> Result = *(static_cast<Token<std::string>*> (v.get()));
            std::cout << Result.Value;
        }
    }

    return std::shared_ptr<Token<std::string>>(new Token <std::string>("nil", "nil"));
};

std::unordered_map <std::string, std::function<std::shared_ptr<BaseToken>(std::shared_ptr<Operation>)>> FunctionMap
{
    {"ln",CustomLn},
    {"log",CustomLog},
    {"print",CustomPrint}
    
};



std::shared_ptr<Operation> PerformFunction(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = nullptr) { //ADD ERROR CHECKING
    std::string FunctionName = static_cast<Token<std::string>*>(CurrentParser->CurrentToken.get())->Value;
    CurrentParser->ParseToken("function");
    CurrentParser->ParseToken("(");
    std::shared_ptr<Operation> InputArgs = std::shared_ptr<Operation>(new Operation("tuple", std::vector<std::shared_ptr<BaseToken>>{}));
    do {
        std::shared_ptr<Operation> CurrentOperation = CurrentParser->SyntaxTraverse();
        InputArgs->Arguments.push_back(Interpret(CurrentOperation));
    } while (CurrentParser->ParseIfExists(","));

    CurrentParser->ParseToken(")");
    std::shared_ptr<BaseToken> Result = FunctionMap[FunctionName](InputArgs);
    return CurrentParser->SyntaxTraverse(ConvertToBase(std::shared_ptr<Operation>(new Operation(Result->Type, std::vector<std::shared_ptr<BaseToken>>{Result}))));
};

std::shared_ptr<Operation> PerformConditional(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = nullptr) { //ADD ERROR CHECKING
    std::string FunctionName = static_cast<Token<std::string>*>(CurrentParser->CurrentToken.get())->Value;
    CurrentParser->ParseToken("if");
    std::shared_ptr<Operation> CurrentCondition = CurrentParser->SyntaxTraverse();
    CurrentParser->ParseToken("then");
    if (true) { //Change it to form a conditional statement
        //Convert Tokens back to text and re-iterate over them
        //Also, sandbox variables!

    }
    else {
        
    }
    while (CurrentParser->ParseIfExists("then")) {

    }
    CurrentParser->ParseToken("end");
    //std::shared_ptr<BaseToken> Result = FunctionMap[FunctionName](InputArgs);
    return CurrentParser->SyntaxTraverse(ConvertToBase(std::shared_ptr<Operation>(new Operation("PLACEHOLDER", std::vector<std::shared_ptr<BaseToken>>{}))));
};



int main()
{
    ProcessMap = {
        {"(",LParen},
        {"*",Mult},
        {"/",Divide},
        {"+",Plus},
        {"-",Minus},
        {"^",RaisePower},
        {"and",PerformAnd},
        {"or",PerformOr},
        {"==",PerformEquivalent},
        //Built-in functions below

        {"function",PerformFunction}
    };



    //OFFICIALLY LEVEL 2.5!
    std::string Text = "print(\"Hi, my name is Mark and I am \", false and 19 or \"unknown\", \" years old!\")";
    Parser NewParser(Text);
    NewParser.SyntaxTraverse();
    //std::shared_ptr<BaseToken> Result = Interpret(NewParser.SyntaxTraverse());
    //Token<double> NewResult = *(static_cast<Token<double>*>(std::move(Result).get()));
    //std::cout << NewResult.Value;
    //Start up the stuff once fully implemented
}
