#ifndef HDR_HDR_GRULES
#define HDR_HDR_GRULES

#include <string>
#include <vector>
#include <list>

#include "lattice.hpp"

// uwaga! GNU C++ i Builder maj� r�ne pogl�dy nt. kolejno�ci deklaracji

using std::string;
using std::ostream;
using std::istream;

using namespace std;

struct GRule;


struct GRuleList: public vector<GRule*>
{
    ~GRuleList();
};


struct GRuleRightSymbolAltsSeq;
struct GRuleRightSymbolsSeq: public vector<GRuleRightSymbolAltsSeq*>
{
    ~GRuleRightSymbolsSeq();
};

struct GRuleRightSymbolAltsSeq
{
    string* atom;
    bool is_starred;
    bool is_optional;
    vector<GRuleRightSymbolsSeq*> alts;

    GRuleRightSymbolAltsSeq():atom(NULL), is_starred(false),
    is_optional(false) {}
    ~GRuleRightSymbolAltsSeq();
};




// ==================================================================

struct GRuleTreeRecipePathNode
{
    string* category;
    string* label;

    enum ESearchType
    {
    ONLY_SEARCH,
    SEARCH_AND_CREATE
    };

    ESearchType search_type;

    GRuleTreeRecipePathNode():category(NULL), label(NULL), search_type(ONLY_SEARCH)
    {}

    ~GRuleTreeRecipePathNode()
    { delete category; delete label;}
};

typedef list<GRuleTreeRecipePathNode*> GRuleTreeRecipePath;

struct GRuleTreeRecipe
{
    string* category;
    string* label;
    string* symbol;
    int number;
    GRuleTreeRecipePath* path;
    list<GRuleTreeRecipe*>* subrecipes;
    enum EHowToInsert
    {
    INSERT_BEFORE,
    INSERT_AFTER,
    ONLY_PUT
    };

    EHowToInsert how_to_insert;

    bool is_contents_ref;

    GRuleTreeRecipe():category(NULL),label(NULL),symbol(NULL), number(0),
    path(NULL),subrecipes(NULL),how_to_insert(INSERT_AFTER),
    is_contents_ref(false) {}
    ~GRuleTreeRecipe();
};

// ==================================================================

struct GRuleExpression
{
    enum EExpressionType
    {
    NUMBER,
    ATOM,
    ATTRIBUTE,
    OPERATOR_MONO,
    OPERATOR_BI,
    TREE_CHOICE,
    SETSCORE
    };

    enum EOperator
    {
    NO_OPERATOR,
    OPERATOR_AND,
    OPERATOR_ASSIGN,
    OPERATOR_EQUAL,
    OPERATOR_NOT_EQUAL,
    OPERATOR_OR,
    OPERATOR_ALWAYS_TRUE,
    OPERATOR_SEMANTICS_INTERSECTION,
    OPERATOR_UASSIGN,
    OPERATOR_UEQUAL,
    OPERATOR_HOOKING,
    OPERATOR_IMPLICATION,
    OPERATOR_PRINT,
    OPERATOR_ELSE,
    OPERATOR_SUM,
    OPERATOR_BINARG
    };

    EExpressionType expr_type;
    EOperator expr_operator;

    int number;
    string* atom;
    string* attribute;
    Lattice::Score score;
    GRuleTreeRecipe* tree_choice;

    GRuleExpression* left_subexpr;
    GRuleExpression* right_subexpr;

    GRuleExpression(EExpressionType a_expr_type = NUMBER):
        expr_type(a_expr_type), expr_operator(NO_OPERATOR),
    number(0), atom(NULL), attribute(NULL),
    score(0.0), tree_choice(NULL),
    left_subexpr(NULL), right_subexpr(NULL) {}

    ~GRuleExpression()
    {
        delete right_subexpr;
        delete left_subexpr;
        delete tree_choice;
           delete atom; delete attribute;
    }
};

struct GRule
{
    string* left_symbol;
    GRuleRightSymbolsSeq* right_symbols;
    GRuleExpression* expr;
    GRuleExpression* extra_expr;
    GRuleTreeRecipe* recipe;

    GRule():left_symbol(NULL), right_symbols(NULL), expr(NULL), extra_expr(0) {}
    ~GRule();
};

#endif
