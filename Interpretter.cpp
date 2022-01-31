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
#include <fstream>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <iterator>

//TODO: Implement variable/function hashmap
class Parser;
class Lexer;
class Operation;
class BaseToken;
std::shared_ptr<BaseToken> CopyToken(std::shared_ptr<BaseToken> const CurrentToken);
//REMEMBER TO DESTROY THE ENTIRE SCOPE AFTER IT EXITS THE SCOPE!!!!!
std::unordered_map <std::string, int> PriorityMap
{
    {"=",11},
    {"local",1},
    {"for",1},
    {"while",1},
    {"function",2},
    {"if",1},
    {"(",3},
    {"^",4},
    {"*",5},
    {"/",5},
    {"+",6},
    {"-",6},
    {"<",7},
    {">",7},
    {"<=",7},
    {">=",7},
    {"==",8},
    {"and",9},
    {"or",10},
};

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
    //std::vector<std::string> MemberTypes;
    std::vector<std::shared_ptr<BaseToken>> Arguments;
    Operation(std::string OperationInp, std::vector<std::shared_ptr<BaseToken>> Args) {
        this->Arguments = Args;
        this->Type = OperationInp;
        
        int TotalArgs = Args.size();
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

std::vector<Token<std::string>*> Tokens = { new Token <std::string>("~=","~="),new Token <std::string>("<=","<="),new Token <std::string>(">=",">="),new Token <std::string>(">",">"),new Token <std::string>("<","<"),new Token <std::string>("==","=="),new Token <std::string>(",",","),new Token <std::string>("=","="),new Token <std::string>("*","*"),new Token <std::string>("-","-"),new Token <std::string>("+","+"),new Token <std::string>("/","/"),new Token <std::string>("(","("),new Token <std::string>(")",")"),new Token <std::string>("^","^") };
std::vector<Token<std::string>*> KeywordTokens = { new Token <std::string>("functionDeclare","function"),new Token <std::string>("do","do"),new Token <std::string>("for","for"),new Token <std::string>("local","local"),new Token <std::string>("elseif","elseif"),new Token <std::string>("else","else"),new Token <std::string>("and","and"),new Token <std::string>("or","or"),new Token <std::string>("true","true"), new Token <std::string>("false","false"), new Token <std::string>("then","then"),new Token <std::string>("function","print"),new Token <std::string>("function","log"),new Token <std::string>("function","ln"),new Token <std::string>("if","if"),new Token <std::string>("end","end"),new Token <std::string>("while","while"),new Token <std::string>("nil","nil") };

//Refers to Tokens which require no left nodes during operations
std::unordered_map <std::string, bool> LValueTokens
{
    {"function",true},
    {"variable",true},
    {"if",true},
    {"while",true},
    {"for",true},
    {"local",true}
};

class Lexer {
    int Position = 0;
    std::string Text;
    char CurrentCharacter;
public:
    int CurrentLine = 0;
    int Scope = 1;

    Lexer(std::string TextInp) {
        this->Text = TextInp;
        this->CurrentCharacter = this->Text[Position];
    };
    std::vector<std::shared_ptr<BaseToken>> TokenizeScript() {
        std::vector<std::shared_ptr<BaseToken>> List;
        while (this->Position < Text.size()) {
            List.push_back(NextToken());
        }
        return List;
    }
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
    char TestForQuotations(char const character) {
        if (character == '\'') {
            return '\'';
        }
        if (character == '\"') {
            return '\"';
        }
        return '\0';
    }

    //ADD HASHMAPS FOR SPEED BOOST IN FUTURE!
    bool NonLetterCheck(char const Character) {
        std::string AllLetters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890";
        for (int i = 0; i < AllLetters.size(); i++) {
            if (AllLetters[i] == Character) {
                return false;
            }
        }
    return true;
    }
    std::string VarCheck() {
        std::string Result = "";

        while (!(this->NonLetterCheck(this->CurrentCharacter))) {
            Result += this->CurrentCharacter;
            this->Advance();
        }

        return Result;
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
                if (TokenCompare(KeywordTokens[i]) && ((this->Position + (*KeywordTokens[i]).Value.size() == this->Text.size() - 1) || (this->Position + (*KeywordTokens[i]).Value.size() + 1 < this->Text.size() && NonLetterCheck(this->Text[this->Position + (*KeywordTokens[i]).Value.size()])))) {
                    BaseToken* NewToken = static_cast<BaseToken*>(new Token<std::string>(KeywordTokens[i]->Type, KeywordTokens[i]->Value, KeywordTokens[i]->SubType));
                    MultiAdvance(KeywordTokens[i]->Value.size());
                    return std::shared_ptr<BaseToken>(NewToken);
                }
            }

            //Check For Variables
            if (!NonLetterCheck(this->Text[this->Position])) {//VarCheck()
                std::string VarName = VarCheck();
                BaseToken* NewToken = static_cast<BaseToken*>(new Token<std::string>("variable", VarName, "variable"));

                return std::shared_ptr<BaseToken>(NewToken);
            }

            std::string ErrorMessage = static_cast<std::string>("Error on line ");
            ErrorMessage += (std::to_string(this->CurrentLine) + static_cast<std::string>(": Illegal symbol \'") + this->CurrentCharacter + static_cast<std::string>("\' found."));
            this->RaiseException(ErrorMessage);
        }
        BaseToken* NewToken = new Token<char>("EOF", '\0');
        return std::shared_ptr<BaseToken>(NewToken);
    }
};

std::unordered_map <std::string, std::function<std::shared_ptr<Operation>(Parser*, std::shared_ptr<BaseToken>)>> ProcessMap;
std::unordered_map <int, std::unordered_map<std::string, std::shared_ptr<BaseToken>>> Environment;

//Causes variables to default to nil
std::shared_ptr<BaseToken>& std::unordered_map<std::string, std::shared_ptr<BaseToken>>::operator[](const std::string& Index) {
    auto Iterator = this->find(Index);
    if (Iterator != this->end()) {
        return Iterator->second;
    }
    else {
        this->insert({ Index,std::shared_ptr<BaseToken>(new Token <std::string>("nil", "nil")) });
        return this->at(Index);
    }
}

std::unordered_map<std::string, std::shared_ptr<BaseToken>>& std::unordered_map<int, std::unordered_map<std::string, std::shared_ptr<BaseToken>>>::operator[](const int& Index) {
    auto Iterator = this->find(Index);
    if (Iterator != this->end()) {
        return Iterator->second;
    }
    else {
        this->insert({ Index,std::unordered_map<std::string, std::shared_ptr<BaseToken>>({}) });
        return this->at(Index);
    }
}

std::shared_ptr<Operation> VariableToValue(std::shared_ptr<Operation> CurrentOperation,int const Scope) {
    std::vector<std::shared_ptr<BaseToken>> Args;
    
    for (std::shared_ptr<BaseToken> v : CurrentOperation->Arguments) {\
        std::shared_ptr<BaseToken> NewArg;
        if (v->Type == "variable") {
            std::string Key = static_cast<Token<std::string>*>(v.get())->Value;
            std::shared_ptr<BaseToken> LookUpResult;
            for (int i = Scope; i >= 0; i--) {
                LookUpResult = CopyToken(Environment[i][Key]);
                if (LookUpResult->Type != "nil") {
                    break;
                }
            }
            NewArg = LookUpResult;
        }
        else {
            NewArg = CopyToken(v);
        }
        Args.push_back(NewArg);
    }
    //std::cout << CurrentOperation->Type;
    return std::shared_ptr<Operation>(new Operation(CurrentOperation->Type, Args));
}

bool CheckVarToken(std::shared_ptr<BaseToken> CurrentToken, int const Scope) {
    std::string Key = static_cast<Token<std::string>*>(CurrentToken.get())->Value;
    for (int i = Scope; i >= 0; i--) {
        if (Environment[i][Key]->Type != "nil") {
            return true;
        }
    }
    return false;
}

std::shared_ptr<BaseToken> CopyToken(std::shared_ptr<BaseToken> const CurrentToken) {
    if (CurrentToken->Type == "number") {
        std::shared_ptr<BaseToken> NewToken = std::shared_ptr<Token<double>>(new Token<double>(CurrentToken->Type, static_cast<Token<double>*>(CurrentToken.get())->Value, CurrentToken->SubType));
        return NewToken;
    }
    else {
        std::shared_ptr<BaseToken> NewToken = std::shared_ptr<Token<std::string>>(new Token<std::string>(CurrentToken->Type, static_cast<Token<std::string>*>(CurrentToken.get())->Value, CurrentToken->SubType));
        return NewToken;
    }
    //Set up function copying!!!
}
//Interpret operations
std::shared_ptr<BaseToken> Interpret(std::shared_ptr<Operation> InputOperation,int const Scope, bool const IsLocal = false) {

    //Low level operation/Tokens
    std::vector<std::string> BaseTokens = { "number","string","nil","true","false","variable"}; //Variables must be resolved

    for (int i = 0; i < BaseTokens.size(); i++) {
        if ((*InputOperation).Type == BaseTokens[i]) {
            std::shared_ptr<BaseToken> NewToken = InputOperation->Arguments[0];
            return NewToken;
        }
    }
    
    if ((*InputOperation).Type == "=") {
        std::string Key = static_cast<Token<std::string>*>(InputOperation->Arguments[0].get())->Value;
        if (IsLocal) {
            Environment[Scope][Key] = CopyToken(InputOperation->Arguments[1]);
        }
        else { //Check if initial declaration was nil later on
            if (CheckVarToken(InputOperation->Arguments[0],Scope)) {
                for (int i = Scope; i >= 0; i--) {
                    if (Environment[i][Key]->Type != "nil") { //Later check if it was originally undeclared
                        Environment[i][Key] = CopyToken(InputOperation->Arguments[1]);
                    }
                }
            }
            else {
                Environment[Scope][Key] = CopyToken(InputOperation->Arguments[1]);
            }
        }
        return std::shared_ptr<Token<std::string>>(new Token <std::string>("nil", "nil"));
    }

    //Operation Implementation

    //All Operations below are after Variable to Value Operations!!!
    std::shared_ptr<Operation> CurrentOperation = VariableToValue(InputOperation,Scope);


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
            BaseToken* NewToken = new Token<std::string>("false", "false");
            return std::shared_ptr<BaseToken>(NewToken);
        }
    }
    if ((*CurrentOperation).Type == "~=") {
        if (CurrentOperation->Arguments[0].get()->Type != CurrentOperation->Arguments[1].get()->Type) {
            BaseToken* NewToken = new Token<std::string>("true", "true");
            return std::shared_ptr<BaseToken>(NewToken);
        }
        else {
            if (CurrentOperation->Arguments[0].get()->Type == "true" || CurrentOperation->Arguments[1].get()->Type == "false" || CurrentOperation->Arguments[1].get()->Type == "nil") {
                BaseToken* NewToken = new Token<std::string>("false", "false");
                return std::shared_ptr<BaseToken>(NewToken);
            }
            if (CurrentOperation->Arguments[0].get()->Type == "number" && (static_cast<Token<double>*>(CurrentOperation->Arguments[0].get())->Value == static_cast<Token<double>*>(CurrentOperation->Arguments[1].get())->Value)) {
                BaseToken* NewToken = new Token<std::string>("false", "false");
                return std::shared_ptr<BaseToken>(NewToken);
            }
            if (CurrentOperation->Arguments[0].get()->Type == "string" && (static_cast<Token<std::string>*>(CurrentOperation->Arguments[0].get())->Value == static_cast<Token<std::string>*>(CurrentOperation->Arguments[1].get())->Value)) {
                BaseToken* NewToken = new Token<std::string>("false", "false");
                return std::shared_ptr<BaseToken>(NewToken);
            }
            BaseToken* NewToken = new Token<std::string>("true", "true");
            return std::shared_ptr<BaseToken>(NewToken);
        }
    }
    if ((*CurrentOperation).Type == "<") { //ERROR IF INVALID TYPING!!!!! ALSO CONVERT STRINGS TO SIZES!!!!
        if (CurrentOperation->Arguments[0].get()->Type == CurrentOperation->Arguments[1].get()->Type) {
            if (CurrentOperation->Arguments[0].get()->Type == "number" && (static_cast<Token<double>*>(CurrentOperation->Arguments[0].get())->Value < static_cast<Token<double>*>(CurrentOperation->Arguments[1].get())->Value)) {
                BaseToken* NewToken = new Token<std::string>("true", "true");
                return std::shared_ptr<BaseToken>(NewToken);
            }
            if (CurrentOperation->Arguments[0].get()->Type == "string" && (static_cast<Token<std::string>*>(CurrentOperation->Arguments[0].get())->Value.size() < static_cast<Token<std::string>*>(CurrentOperation->Arguments[1].get())->Value.size())) {
                BaseToken* NewToken = new Token<std::string>("true", "true");
                return std::shared_ptr<BaseToken>(NewToken);
            }
            BaseToken* NewToken = new Token<std::string>("false", "false");
            return std::shared_ptr<BaseToken>(NewToken);
        }
        // ERROR IF INVALID TYPING!!!!!
    }
    if ((*CurrentOperation).Type == ">") { //ERROR IF INVALID TYPING!!!!! ALSO CONVERT STRINGS TO SIZES!!!!
        if (CurrentOperation->Arguments[0].get()->Type == CurrentOperation->Arguments[1].get()->Type) {
            if (CurrentOperation->Arguments[0].get()->Type == "number" && (static_cast<Token<double>*>(CurrentOperation->Arguments[0].get())->Value > static_cast<Token<double>*>(CurrentOperation->Arguments[1].get())->Value)) {
                BaseToken* NewToken = new Token<std::string>("true", "true");
                return std::shared_ptr<BaseToken>(NewToken);
            }
            if (CurrentOperation->Arguments[0].get()->Type == "string" && (static_cast<Token<std::string>*>(CurrentOperation->Arguments[0].get())->Value.size() > static_cast<Token<std::string>*>(CurrentOperation->Arguments[1].get())->Value.size())) {
                BaseToken* NewToken = new Token<std::string>("true", "true");
                return std::shared_ptr<BaseToken>(NewToken);
            }
            BaseToken* NewToken = new Token<std::string>("false", "false");
            return std::shared_ptr<BaseToken>(NewToken);
        }
        // ERROR IF INVALID TYPING!!!!!
    }
    if ((*CurrentOperation).Type == "<") { //ERROR IF INVALID TYPING!!!!! ALSO CONVERT STRINGS TO SIZES!!!!
        if (CurrentOperation->Arguments[0].get()->Type == CurrentOperation->Arguments[1].get()->Type) {
            if (CurrentOperation->Arguments[0].get()->Type == "number" && (static_cast<Token<double>*>(CurrentOperation->Arguments[0].get())->Value <= static_cast<Token<double>*>(CurrentOperation->Arguments[1].get())->Value)) {
                BaseToken* NewToken = new Token<std::string>("true", "true");
                return std::shared_ptr<BaseToken>(NewToken);
            }
            if (CurrentOperation->Arguments[0].get()->Type == "string" && (static_cast<Token<std::string>*>(CurrentOperation->Arguments[0].get())->Value.size() <= static_cast<Token<std::string>*>(CurrentOperation->Arguments[1].get())->Value.size())) {
                BaseToken* NewToken = new Token<std::string>("true", "true");
                return std::shared_ptr<BaseToken>(NewToken);
            }
            BaseToken* NewToken = new Token<std::string>("false", "false");
            return std::shared_ptr<BaseToken>(NewToken);
        }
        // ERROR IF INVALID TYPING!!!!!
    }
    if ((*CurrentOperation).Type == ">=") { //ERROR IF INVALID TYPING!!!!! ALSO CONVERT STRINGS TO SIZES!!!!
        if (CurrentOperation->Arguments[0].get()->Type == CurrentOperation->Arguments[1].get()->Type) {
            if (CurrentOperation->Arguments[0].get()->Type == "number" && (static_cast<Token<double>*>(CurrentOperation->Arguments[0].get())->Value >= static_cast<Token<double>*>(CurrentOperation->Arguments[1].get())->Value)) {
                BaseToken* NewToken = new Token<std::string>("true", "true");
                return std::shared_ptr<BaseToken>(NewToken);
            }
            if (CurrentOperation->Arguments[0].get()->Type == "string" && (static_cast<Token<std::string>*>(CurrentOperation->Arguments[0].get())->Value.size() >= static_cast<Token<std::string>*>(CurrentOperation->Arguments[1].get())->Value.size())) {
                BaseToken* NewToken = new Token<std::string>("true", "true");
                return std::shared_ptr<BaseToken>(NewToken);
            }
            BaseToken* NewToken = new Token<std::string>("false", "false");
            return std::shared_ptr<BaseToken>(NewToken);
        }
        // ERROR IF INVALID TYPING!!!!!
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
    std::vector<std::shared_ptr<BaseToken>> AllTokensParsed;
public:
    int CurrentTokenPosition = 0;
    std::shared_ptr<BaseToken> CurrentToken;
    int Scope = 1;
    Parser(std::string Text, std::vector<std::shared_ptr<BaseToken>> TokenList = std::vector<std::shared_ptr<BaseToken>>{}) : CurrentLexer(Text) {
        if (TokenList.size() != 0) {
            this->AllTokensParsed = TokenList;
            this->CurrentToken = GetCurrentToken();
        }
        else {
            this->AllTokensParsed = this->CurrentLexer.TokenizeScript();
            this->CurrentToken = GetCurrentToken();
        }
        //Optimize in the future by combining constant terms and pre-indexing token parsing
    }
    void Reset() {
        this->CurrentTokenPosition = 0;
        this->CurrentToken = GetCurrentToken();
    }
    void ParseToken(std::string TokenType) {
        if ((*(this->CurrentToken)).Type == TokenType) {
            this->CurrentToken = GetCurrentToken();
        }
        else { //Convert to error tokens for pcall functionality!
            std::string ErrorMessage = static_cast<std::string>("Error on line ");
            ErrorMessage += (std::to_string(this->CurrentLexer.CurrentLine) + static_cast<std::string>(": Unexpected token \'") + (*(this->CurrentToken)).Type + static_cast<std::string>("\' found when \'") + TokenType + static_cast<std::string>("\' expected."));
            this->CurrentLexer.RaiseException(ErrorMessage);
        }
    }
    void SkipToken() {
        ParseToken((*(this->CurrentToken)).Type);
    }
    std::shared_ptr<BaseToken> GetCurrentToken() {
        if (this->CurrentTokenPosition < AllTokensParsed.size()) {
            return this->AllTokensParsed[this->CurrentTokenPosition++];
        }
        else {
            return std::shared_ptr<BaseToken>(new Token<char>("EOF", '\0')); //TEMPORARY UNTIL EOF IS LOOKED INTO
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
            Operation* NewOperation = new Operation("nil", std::vector<std::shared_ptr<BaseToken>>{(this->CurrentToken)});
            this->ParseToken("nil");
            return std::shared_ptr<Operation>(NewOperation);
        }
        if ((*(this->CurrentToken)).Type == "variable") {
            Operation* NewOperation = new Operation("variable", std::vector<std::shared_ptr<BaseToken>>{(this->CurrentToken)});
            this->ParseToken("variable");
            return std::shared_ptr<Operation>(NewOperation);
        }
        return std::shared_ptr<Operation>(nullptr);
    }
    std::shared_ptr<Operation> SyntaxTraverse(std::shared_ptr<Operation> NodeInp = std::shared_ptr<Operation>(nullptr),const bool Override = false) { //Third Parameter may be unused
        std::shared_ptr<Operation> FirstNode = NodeInp;

        //Allows for LValue tokens to be treated as if there was no first node
        if (NodeInp.get() != nullptr && LValueTokens[(*(this->CurrentToken)).Type] == true && PriorityMap[NodeInp->Type] == 0) {
            FirstNode = std::shared_ptr<Operation>(nullptr);
        }

        if (FirstNode.get() == nullptr) {
            FirstNode = this->ReturnBaseTokens();
        }

        //Allows for LValue tokens to be treated as if there was no first node
        if (FirstNode.get() != nullptr && LValueTokens[(*(this->CurrentToken)).Type] == true) {
            return FirstNode;
        }
        if (FirstNode.get() == nullptr && PriorityMap[(*(this->CurrentToken)).Type] != 0) {
            FirstNode = ProcessMap[(*(this->CurrentToken)).Type](this, std::shared_ptr<BaseToken>(nullptr));
        }
        if ((this->CurrentToken)->Type == "EOF" || (PriorityMap[(*(this->CurrentToken)).Type] == 0) ||(Override && PriorityMap[FirstNode->Type] == 0)) {
            if (FirstNode.get() == nullptr) { //Possibly Unused
                return std::shared_ptr<Operation>(new Operation("nil", std::vector<std::shared_ptr<BaseToken>>{ std::shared_ptr<Token<std::string>>(new Token <std::string>("nil", "nil")) }));
            }
            return FirstNode;
        }
        if (NodeInp.get() == nullptr || (PriorityMap[NodeInp->Type] <= PriorityMap[(*(this->CurrentToken)).Type])) {
            std::shared_ptr<BaseToken> NewFirstNode = Interpret(FirstNode,this->Scope);
            return ProcessMap[(*(this->CurrentToken)).Type](this, (NewFirstNode));
        }
        else {
            Operation* NewNode = new Operation(NodeInp->Type, std::vector<std::shared_ptr<BaseToken>>{(FirstNode->Arguments[0]), Interpret(ProcessMap[(*(this->CurrentToken)).Type](this, (FirstNode->Arguments[1])),this->Scope)});
            return (this->SyntaxTraverse(std::shared_ptr<Operation>(NewNode)));
        }
    }
};

std::shared_ptr<Operation> ConvertToBase(std::shared_ptr<Operation> NewOperation, Parser* CurrentParser) {
    std::shared_ptr<BaseToken> NewToken = Interpret(NewOperation, CurrentParser->Scope);
    return std::shared_ptr<Operation>(new Operation(NewToken->Type, std::vector<std::shared_ptr<BaseToken>>{ NewToken }));
};

//Lowest Level Token Interpretations
std::shared_ptr<Operation> LParen(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = std::shared_ptr<BaseToken>(nullptr)) {
    CurrentParser->ParseToken("(");
    std::shared_ptr<Operation> CurrentOperation = CurrentParser->SyntaxTraverse();
    CurrentParser->ParseToken(")");
    return CurrentParser->SyntaxTraverse(ConvertToBase(CurrentOperation, CurrentParser)); // Take prior stuff
};

std::shared_ptr<Operation> Mult(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = std::shared_ptr<BaseToken>(nullptr)) { //ADD ERROR CHECKING
    std::shared_ptr<BaseToken> FirstNode = (Node);
    CurrentParser->ParseToken("*");
    std::shared_ptr<Operation> SecondNodeOP = CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(nullptr),true);
    std::shared_ptr<BaseToken> SecondNode = Interpret(SecondNodeOP, CurrentParser->Scope);
    return CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(new Operation("*", std::vector<std::shared_ptr<BaseToken>>{(FirstNode), (SecondNode)})));
};

std::shared_ptr<Operation> Plus(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = std::shared_ptr<BaseToken>(nullptr)) { //ADD ERROR CHECKING
    std::shared_ptr<BaseToken> FirstNode = (Node);
    CurrentParser->ParseToken("+");
    std::shared_ptr<Operation> SecondNodeOP = CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(nullptr), true);
    std::shared_ptr<BaseToken> SecondNode = Interpret(SecondNodeOP, CurrentParser->Scope);
    return CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(new Operation("+", std::vector<std::shared_ptr<BaseToken>>{(FirstNode), (SecondNode)})));
};

std::shared_ptr<Operation> Minus(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = std::shared_ptr<BaseToken>(nullptr)) { //ADD ERROR CHECKING
    std::shared_ptr<BaseToken> FirstNode = (Node);
    CurrentParser->ParseToken("-");
    std::shared_ptr<Operation> SecondNodeOP = CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(nullptr), true);
    std::shared_ptr<BaseToken> SecondNode = Interpret(SecondNodeOP, CurrentParser->Scope);
    return CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(new Operation("-", std::vector<std::shared_ptr<BaseToken>>{(FirstNode), (SecondNode)})));
};

std::shared_ptr<Operation> Divide(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = nullptr) { //ADD ERROR CHECKING
    std::shared_ptr<BaseToken> FirstNode = (Node);
    CurrentParser->ParseToken("/");
    std::shared_ptr<Operation> SecondNodeOP = CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(nullptr), true);
    std::shared_ptr<BaseToken> SecondNode = Interpret(SecondNodeOP, CurrentParser->Scope);
    return CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(new Operation("/", std::vector<std::shared_ptr<BaseToken>>{(FirstNode), (SecondNode)})));
};

std::shared_ptr<Operation> RaisePower(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = nullptr) { //ADD ERROR CHECKING
    std::shared_ptr<BaseToken> FirstNode = (Node);
    CurrentParser->ParseToken("^");
    std::shared_ptr<Operation> SecondNodeOP = CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(nullptr), true);
    std::shared_ptr<BaseToken> SecondNode = Interpret(SecondNodeOP, CurrentParser->Scope);
    return CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(new Operation("^", std::vector<std::shared_ptr<BaseToken>>{(FirstNode), (SecondNode)})));
};

std::shared_ptr<Operation> PerformAnd(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = nullptr) { //ADD ERROR CHECKING
    std::shared_ptr<BaseToken> FirstNode = (Node);
    CurrentParser->ParseToken("and");
    std::shared_ptr<Operation> SecondNodeOP = CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(nullptr), true);
    std::shared_ptr<BaseToken> SecondNode = Interpret(SecondNodeOP, CurrentParser->Scope);
    return CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(new Operation("and", std::vector<std::shared_ptr<BaseToken>>{(FirstNode), (SecondNode)})));
};

std::shared_ptr<Operation> PerformOr(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = nullptr) { //ADD ERROR CHECKING
    std::shared_ptr<BaseToken> FirstNode = (Node);
    CurrentParser->ParseToken("or");
    std::shared_ptr<Operation> SecondNodeOP = CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(nullptr), true);
    std::shared_ptr<BaseToken> SecondNode = Interpret(SecondNodeOP, CurrentParser->Scope);
    return CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(new Operation("or", std::vector<std::shared_ptr<BaseToken>>{(FirstNode), (SecondNode)})));
};

std::shared_ptr<Operation> PerformEquivalent(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = nullptr) { //ADD ERROR CHECKING
    std::shared_ptr<BaseToken> FirstNode = (Node);
    CurrentParser->ParseToken("==");
    std::shared_ptr<Operation> SecondNodeOP = CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(nullptr), true);
    std::shared_ptr<BaseToken> SecondNode = Interpret(SecondNodeOP, CurrentParser->Scope);
    return CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(new Operation("==", std::vector<std::shared_ptr<BaseToken>>{(FirstNode), (SecondNode)})));
};

std::shared_ptr<Operation> PerformNotEquivalent(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = nullptr) { //ADD ERROR CHECKING
    std::shared_ptr<BaseToken> FirstNode = (Node);
    CurrentParser->ParseToken("~=");
    std::shared_ptr<Operation> SecondNodeOP = CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(nullptr), true);
    std::shared_ptr<BaseToken> SecondNode = Interpret(SecondNodeOP, CurrentParser->Scope);
    return CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(new Operation("~=", std::vector<std::shared_ptr<BaseToken>>{(FirstNode), (SecondNode)})));
};

std::shared_ptr<Operation> PerformLessThan(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = nullptr) { //ADD ERROR CHECKING
    std::shared_ptr<BaseToken> FirstNode = (Node);
    CurrentParser->ParseToken("<");
    std::shared_ptr<Operation> SecondNodeOP = CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(nullptr), true);
    std::shared_ptr<BaseToken> SecondNode = Interpret(SecondNodeOP, CurrentParser->Scope);
    return CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(new Operation("<", std::vector<std::shared_ptr<BaseToken>>{(FirstNode), (SecondNode)})));
};

std::shared_ptr<Operation> PerformGreaterThan(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = nullptr) { //ADD ERROR CHECKING
    std::shared_ptr<BaseToken> FirstNode = (Node);
    CurrentParser->ParseToken(">");
    std::shared_ptr<Operation> SecondNodeOP = CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(nullptr), true);
    std::shared_ptr<BaseToken> SecondNode = Interpret(SecondNodeOP, CurrentParser->Scope);
    return CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(new Operation(">", std::vector<std::shared_ptr<BaseToken>>{(FirstNode), (SecondNode)})));
};

std::shared_ptr<Operation> PerformLessOrEqual(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = nullptr) { //ADD ERROR CHECKING
    std::shared_ptr<BaseToken> FirstNode = (Node);
    CurrentParser->ParseToken("<=");
    std::shared_ptr<Operation> SecondNodeOP = CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(nullptr), true);
    std::shared_ptr<BaseToken> SecondNode = Interpret(SecondNodeOP, CurrentParser->Scope);
    return CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(new Operation("<=", std::vector<std::shared_ptr<BaseToken>>{(FirstNode), (SecondNode)})));
};

std::shared_ptr<Operation> PerformGreaterOrEqual(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = nullptr) { //ADD ERROR CHECKING
    std::shared_ptr<BaseToken> FirstNode = (Node);
    CurrentParser->ParseToken(">=");
    std::shared_ptr<Operation> SecondNodeOP = CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(nullptr), true);
    std::shared_ptr<BaseToken> SecondNode = Interpret(SecondNodeOP, CurrentParser->Scope);
    return CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(new Operation(">=", std::vector<std::shared_ptr<BaseToken>>{(FirstNode), (SecondNode)})));
};

std::shared_ptr<Operation> PerformEquate(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = nullptr) { //ADD ERROR CHECKING
    std::shared_ptr<BaseToken> FirstNode = (Node);
    CurrentParser->ParseToken("=");
    std::shared_ptr<Operation> SecondNodeOP = VariableToValue(CurrentParser->SyntaxTraverse(), CurrentParser->Scope);
    std::shared_ptr<BaseToken> SecondNode = Interpret(SecondNodeOP, CurrentParser->Scope);
    Interpret(std::shared_ptr<Operation>(new Operation("=", std::vector<std::shared_ptr<BaseToken>>{(FirstNode), (SecondNode)})),CurrentParser->Scope);
    return CurrentParser->SyntaxTraverse();
};

std::shared_ptr<Operation> PerformLocalEquate(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = nullptr) { //ADD ERROR CHECKING
    CurrentParser->ParseToken("local");
    std::shared_ptr<BaseToken> FirstNode = Interpret(CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(nullptr), true), CurrentParser->Scope);
    CurrentParser->ParseToken("=");
    std::shared_ptr<Operation> SecondNodeOP = VariableToValue(CurrentParser->SyntaxTraverse(), CurrentParser->Scope);
    std::shared_ptr<BaseToken> SecondNode = Interpret(SecondNodeOP, CurrentParser->Scope);
    Interpret(std::shared_ptr<Operation>(new Operation("=", std::vector<std::shared_ptr<BaseToken>>{(FirstNode), (SecondNode)})), CurrentParser->Scope,true);
    return CurrentParser->SyntaxTraverse();
};

std::shared_ptr<Operation> PerformFor(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = nullptr) { //ADD ERROR CHECKING
    CurrentParser->ParseToken("for");
    std::shared_ptr<BaseToken> FirstNode = Interpret(CurrentParser->SyntaxTraverse(std::shared_ptr<Operation>(nullptr), true), CurrentParser->Scope);
    CurrentParser->ParseToken("=");
    double Start = static_cast<Token<double>*>(Interpret(VariableToValue(CurrentParser->SyntaxTraverse(), CurrentParser->Scope), CurrentParser->Scope).get())->Value;
    CurrentParser->ParseToken(",");
    double End = static_cast<Token<double>*>(Interpret(VariableToValue(CurrentParser->SyntaxTraverse(), CurrentParser->Scope), CurrentParser->Scope).get())->Value;

    double Step;
    if (CurrentParser->ParseIfExists(",")) {
        Step = static_cast<Token<double>*>(Interpret(VariableToValue(CurrentParser->SyntaxTraverse(), CurrentParser->Scope), CurrentParser->Scope).get())->Value;
    }
    else {
        Step = 1;
    }

    CurrentParser->ParseToken("do");
    std::vector<std::shared_ptr<BaseToken>> ToParseInFor;
    
    std::unordered_map<std::string, bool> TokensWithEnd = { {"do",true},{"if",true},{"functionDeclare",true} }; //Add the "do" token later on for scope increases
    int TotalEnds = 0;
    while (CurrentParser->CurrentToken->Type!="end" || TotalEnds!=0) {
        if (TokensWithEnd[CurrentParser->CurrentToken->Type]) {
            TotalEnds++;
        }
        if (CurrentParser->CurrentToken->Type == "end") {
            TotalEnds--;
        }
        ToParseInFor.push_back(CurrentParser->CurrentToken);
        CurrentParser->ParseToken(CurrentParser->CurrentToken->Type);
    }
    CurrentParser->Scope++;

    Parser NewParser("", ToParseInFor);
    for (double i = Start; i < End; i = i + Step) {

        Interpret(std::shared_ptr<Operation>(new Operation("=", std::vector<std::shared_ptr<BaseToken>>{(FirstNode), std::shared_ptr<BaseToken>(new Token<double>("number",i))})), CurrentParser->Scope,true);
        NewParser.Reset();
        NewParser.Scope = CurrentParser->Scope;
        NewParser.SyntaxTraverse();

    }
    CurrentParser->ParseToken("end");
    Environment[CurrentParser->Scope] = std::unordered_map<std::string, std::shared_ptr<BaseToken>>{};
    CurrentParser->Scope--;
    return CurrentParser->SyntaxTraverse();
};

std::vector<std::shared_ptr<BaseToken>> CopyTokenVector(std::vector<std::shared_ptr<BaseToken>> CurrentVector) {
    std::vector<std::shared_ptr<BaseToken>> NewVector;
    for (std::shared_ptr<BaseToken> v : CurrentVector) {
        NewVector.push_back(CopyToken(v));
    }
    return NewVector;
}
std::shared_ptr<Operation> PerformWhile(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = nullptr) { //ADD ERROR CHECKING
    CurrentParser->ParseToken("while");

    std::vector<std::shared_ptr<BaseToken>> ToParseInWhile;

    while (CurrentParser->CurrentToken->Type != "do") {
        ToParseInWhile.push_back(CurrentParser->CurrentToken);
        CurrentParser->ParseToken(CurrentParser->CurrentToken->Type);
    }
    CurrentParser->ParseToken("do");

    Parser Condition("", ToParseInWhile);
    Condition.Scope = CurrentParser->Scope;
    std::shared_ptr<BaseToken> ConditionValue = Interpret(VariableToValue(Condition.SyntaxTraverse(), CurrentParser->Scope), CurrentParser->Scope);
    std::vector<std::shared_ptr<BaseToken>> ToParseInWhileBlock;

    std::unordered_map<std::string, bool> TokensWithEnd = { {"do",true},{"if",true},{"functionDeclare",true} }; //Add the "do" token later on for scope increases
    int TotalEnds = 0;
    while (CurrentParser->CurrentToken->Type != "end" || TotalEnds != 0) {
        if (TokensWithEnd[CurrentParser->CurrentToken->Type]) {
            TotalEnds++;
        }
        if (CurrentParser->CurrentToken->Type == "end") {
            TotalEnds--;
        }
        ToParseInWhileBlock.push_back(CurrentParser->CurrentToken);
        CurrentParser->ParseToken(CurrentParser->CurrentToken->Type);
    }
    
    Parser NewParser("", ToParseInWhileBlock);
    bool CanRun = (ConditionValue->Type == "string" || ConditionValue->Type == "true" || (ConditionValue->Type == "number" && static_cast<Token<double>*>(ConditionValue.get())->Value != 0));
    while (CanRun) {
        CurrentParser->Scope++;
        NewParser.Reset();
        NewParser.Scope = CurrentParser->Scope;
        NewParser.SyntaxTraverse();
        Environment[CurrentParser->Scope] = std::unordered_map<std::string, std::shared_ptr<BaseToken>>{};
        CurrentParser->Scope--;
        Condition.Reset();
        ConditionValue = Interpret(VariableToValue(Condition.SyntaxTraverse(), CurrentParser->Scope), CurrentParser->Scope);
        CanRun = (ConditionValue->Type == "string" || ConditionValue->Type == "true" || (ConditionValue->Type == "number" && static_cast<Token<double>*>(ConditionValue.get())->Value != 0));
    }

    CurrentParser->ParseToken("end");
    return CurrentParser->SyntaxTraverse();
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
        InputArgs->Arguments.push_back(Interpret(CurrentOperation, CurrentParser->Scope));
    } while (CurrentParser->ParseIfExists(","));

    CurrentParser->ParseToken(")");
    std::shared_ptr<Operation> NewOperation = VariableToValue(InputArgs, CurrentParser->Scope);
    std::shared_ptr<BaseToken> Result = FunctionMap[FunctionName](NewOperation);
    return CurrentParser->SyntaxTraverse(ConvertToBase(std::shared_ptr<Operation>(new Operation(Result->Type, std::vector<std::shared_ptr<BaseToken>>{Result})), CurrentParser));
};

std::shared_ptr<Operation> PerformConditional(Parser* CurrentParser, std::shared_ptr<BaseToken> Node = nullptr) { //ADD ERROR CHECKING
    std::string FunctionName = static_cast<Token<std::string>*>(CurrentParser->CurrentToken.get())->Value;
    CurrentParser->ParseToken("if");
    std::shared_ptr<Operation> CurrentOperation = CurrentParser->SyntaxTraverse();
    VariableToValue(CurrentOperation, CurrentParser->Scope);
    std::shared_ptr<BaseToken> CurrentCondition = Interpret(CurrentOperation, CurrentParser->Scope);
    CurrentParser->ParseToken("then");
    if (CurrentCondition->Type == "string" || CurrentCondition->Type == "true" || (CurrentCondition->Type == "number" && static_cast<Token<double>*>(CurrentCondition.get())->Value != 0)) { //Change it to form a conditional statement
        CurrentParser->Scope++;
        CurrentParser->SyntaxTraverse(); // Remember to introduce a hash table for variable scope!!!
        Environment[CurrentParser->Scope] = std::unordered_map<std::string, std::shared_ptr<BaseToken>>{};
        CurrentParser->Scope--;
    }
    else { //SkipToken()
        while (CurrentParser->CurrentToken->Type != "end") {
            if (CurrentParser->CurrentToken->Type == "elseif") {
                CurrentParser->ParseToken("elseif");
                std::shared_ptr<Operation> CurrentOperation = CurrentParser->SyntaxTraverse();
                VariableToValue(CurrentOperation, CurrentParser->Scope);
                std::shared_ptr<BaseToken> CurrentCondition = Interpret(CurrentOperation, CurrentParser->Scope);
                CurrentParser->ParseToken("then");
                if (CurrentCondition->Type == "string" || CurrentCondition->Type == "true" || (CurrentCondition->Type == "number" && static_cast<Token<double>*>(CurrentCondition.get())->Value != 0)) { //Change it to form a conditional statement
                    CurrentParser->Scope++;
                    CurrentParser->SyntaxTraverse(); // Remember to introduce a hash table for variable scope!!!
                    Environment[CurrentParser->Scope] = std::unordered_map<std::string, std::shared_ptr<BaseToken>>{};
                    CurrentParser->Scope--;
                    break;
                }
            }
            else {
                if (CurrentParser->CurrentToken->Type == "else") {
                    CurrentParser->ParseToken("else");
                    CurrentParser->Scope++;
                    CurrentParser->SyntaxTraverse(); // Remember to introduce a hash table for variable scope!!!
                    Environment[CurrentParser->Scope] = std::unordered_map<std::string, std::shared_ptr<BaseToken>>{};
                    CurrentParser->Scope--;
                    break;
                }
                else {
                    CurrentParser->SkipToken();
                }
            }
        }
        while (CurrentParser->CurrentToken->Type != "end") {
            CurrentParser->SkipToken();
        }
    }
    CurrentParser->ParseToken("end");
    //std::shared_ptr<BaseToken> Result = FunctionMap[FunctionName](InputArgs);
    //return CurrentParser->SyntaxTraverse(ConvertToBase(std::shared_ptr<Operation>(new Operation("PLACEHOLDER", std::vector<std::shared_ptr<BaseToken>>{}))));
    return CurrentParser->SyntaxTraverse();
};

// Use C:\Users\Patchy\Desktop\Level5.txt as test path

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
        {"if",PerformConditional},
        {"=",PerformEquate},
        {"local",PerformLocalEquate},
        {"for",PerformFor},
        {"~=",PerformNotEquivalent},
        {"<",PerformLessThan},
        {">",PerformGreaterThan},
        {"<=",PerformLessOrEqual},
        {">=",PerformGreaterOrEqual},
        {"while",PerformWhile},
        //Built-in functions below

        {"function",PerformFunction}
    };
    std::cout << "File name to read: ";

    std::string FilePath;
    std::cin >> FilePath;
    //FilePath = "C:\\Users\\Patchy\\Desktop\\Level5.txt";
    std::ifstream file{FilePath};

    std::string Text = { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>{} };
    //std::cout << Text;
    std::cout << std::endl;
    //OFFICIALLY LEVEL 3!
    //"if true then print(\"Hi, my name is Mark and I am \", false and 19 or \"unknown\", \" years old!\") end";
    Parser NewParser(Text);
    NewParser.SyntaxTraverse();
    //std::shared_ptr<BaseToken> Result = Interpret(NewParser.SyntaxTraverse(), NewParser);
    //Token<double> NewResult = *(static_cast<Token<double>*>(std::move(Result).get()));
    //std::cout << NewResult.Value;
    //Start up the stuff once fully implemented
}
//REMOVE VARIABLES FROM ALL EXCEPT =
