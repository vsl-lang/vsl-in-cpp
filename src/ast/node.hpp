#ifndef NODE_HPP
#define NODE_HPP

class Node;
class DeclNode;
class FuncInterfaceNode;
class FunctionNode;
class ExtFuncNode;
class ParamNode;
class VariableNode;
class ClassNode;
class FieldNode;
class MethodNode;
class CtorNode;
class BlockNode;
class EmptyNode;
class IfNode;
class ReturnNode;
class ExprNode;
class IdentNode;
class LiteralNode;
class UnaryNode;
class BinaryNode;
class TernaryNode;
class CallNode;
class ArgNode;
class FieldAccessNode;
class MethodCallNode;
class SelfNode;

/**
 * Access specifiers for {@link DeclNode DeclNodes}.
 */
enum class Access
{
    /** Can be accessed anywhere. */
    PUBLIC,
    /** Can be accessed only in the context it was declared. */
    PRIVATE,
    /** Not applicable, e.g. inside a function. */
    NONE
};

#include "ast/nodeVisitor.hpp"
#include "ast/opKind.hpp"
#include "ast/type.hpp"
#include "lexer/location.hpp"
#include "lexer/tokenKind.hpp"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/IR/GlobalValue.h"
#include <string>
#include <vector>

/**
 * Merges two access specifiers. This is used when the grandparent scope wants
 * to access the child. If the parent is private then the child is private, but
 * if the parent is public then the child determines the result.
 *
 * @param parent Access of the parent scope.
 * @param child Access of the child declaration
 *
 * @returns The perceived access specifier to access the child from the parent's
 * parent scope.
 */
Access mergeAccess(Access parent, Access child);

/**
 * Converts a TokenKind keyword to an access specifier. This applies only to the
 * keywords `public` and `private`.
 *
 * @param kind Keyword kind to convert.
 *
 * @returns PUBLIC or PRIVATE depending on the keyword kind given, or NONE if
 * it's not the right keyword kind.
 */
Access keywordToAccess(TokenKind kind);

/**
 * Converts an access specifier to an LLVM linkage. NONE is assumed to be the
 * same as private.
 *
 * @param access Access specifier to convert.
 *
 * @returns External linkage if public, Internal if private.
 */
llvm::GlobalValue::LinkageTypes accessToLinkage(Access access);

/**
 * Base class for the VSL AST.
 */
class Node
{
    friend llvm::raw_ostream& operator<<(llvm::raw_ostream& os,
        const Node& node);
public:
    /**
     * Identifies the kind of Node a given object is. Keep in mind that one
     * subclass can represent multiple Kinds of Nodes.
     */
    enum Kind
    {
        /** Function. */
        FUNCTION,
        /** External function. */
        EXTFUNC,
        /** Function parameter. */
        PARAM,
        /** Variable definition. */
        VARIABLE,
        /** Class definition. */
        CLASS,
        /** Field. */
        FIELD,
        /** Method. */
        METHOD,
        /** Class constructor. */
        CTOR,
        /** Code block. */
        BLOCK,
        /** Empty statement. */
        EMPTY,
        /** If/else statement. */
        IF,
        /** Return statement. */
        RETURN,
        /** Identifier. */
        IDENT,
        /** Integer literal. */
        LITERAL,
        /** Unary expression. */
        UNARY,
        /** Binary expression. */
        BINARY,
        /** Ternary expression. */
        TERNARY,
        /** Function call. */
        CALL,
        /** Function call argument. */
        ARG,
        /** Field access. */
        FIELD_ACCESS,
        /** Method call. */
        METHOD_CALL,
        /** Self keyword. */
        SELF
    };
    /**
     * Creates a Node object.
     *
     * @param kind The kind of Node this is.
     * @param location The source location.
     */
    Node(Kind kind, Location location);
    virtual ~Node() = 0;
    /**
     * Allows a NodeVisitor to visit this object.
     *
     * @param nodeVisitor The visitor object.
     */
    virtual void accept(NodeVisitor& nodeVisitor) = 0;
    /**
     * Verifies whether this Node represents a certain Kind.
     *
     * @param k The Kind to check for.
     *
     * @returns True if this Node represents the given Kind, false otherwise.
     */
    bool is(Kind k) const;
    bool isNot(Kind k) const;
    /**
     * Gets the location info.
     *
     * @returns Where this Node was found in the source.
     */
    Location getLoc() const;
    /**
     * Checks whether this Node is an expression.
     *
     * @returns True if this Node is a subclass of ExprNode, false otherwise.
     */
    virtual bool isExpr() const;

private:
    /** The kind of Node this is. */
    Kind kind;
    /** Where this Node was found in the source. */
    Location location;
};

/**
 * Represents a declaration/definition with an access specifier.
 */
class DeclNode : public Node
{
public:
    /**
     * Creates a DeclNode.
     *
     * @param kind The kind of Node this is.
     * @param location Where this DeclNode was found in the source.
     * @param access Access specifier.
     */
    DeclNode(Node::Kind kind, Location location, Access access);
    /**
     * Gets the access specifier.
     *
     * @returns The access specifier.
     */
    Access getAccess() const;

private:
    /** Access specifier. */
    Access access;
};

/**
 * Represents a function interface, e.g.\ `func f(x: Int) -> Int`. Basically a
 * function without a body, which can be either defined by a FunctionNode or
 * declared external by an ExtFuncNode.
 */
class FuncInterfaceNode : public DeclNode
{
public:
    /**
     * Creates a FuncInterfaceNode.
     *
     * @param kind Node kind.
     * @param location Where this FuncInterfaceNode was found in the source.
     * @param access Access specifier.
     * @param name The name of the function.
     * @param params The function's parameters.
     * @param returnType The type that the function returns.
     */
    FuncInterfaceNode(Node::Kind kind, Location location, Access access,
        llvm::StringRef name, std::vector<ParamNode*> params,
        const Type* returnType);
    llvm::StringRef getName() const;
    llvm::ArrayRef<ParamNode*> getParams() const;
    size_t getNumParams() const;
    ParamNode& getParam(size_t i) const;
    const Type* getReturnType() const;

private:
    /** The name of the function. */
    llvm::StringRef name;
    /** The function's parameters. */
    std::vector<ParamNode*> params;
    /** The function's return type. */
    const Type* returnType;
};

/**
 * Represents a function definition, e.g.\ `func f(x: Int) -> Int { ... }`.
 */
class FunctionNode : public FuncInterfaceNode
{
public:
    /**
     * Creates a FunctionNode.
     *
     * @param location Where this FunctionNode was found in the source.
     * @param access Access specifier.
     * @param name The name of the function.
     * @param params The function's parameters.
     * @param returnType The type that the function returns.
     * @param body The body of the function.
     */
    FunctionNode(Location location, Access access, llvm::StringRef name,
        std::vector<ParamNode*> params, const Type* returnType,
        BlockNode& body);
    virtual ~FunctionNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    BlockNode& getBody() const;
    bool isAlreadyDefined() const;
    void setAlreadyDefined(bool alreadyDefined = true);

protected:
    /**
     * Used by subclasses.
     */
    FunctionNode(Node::Kind kind, Location location, Access access,
        llvm::StringRef name, std::vector<ParamNode*> params,
        const Type* returnType, BlockNode& body);

private:
    /** The body of the function. */
    BlockNode& body;
    /** Whether this function was already defined. */
    bool alreadyDefined;
};

/**
 * Represents an external function, e.g.\ `func f(x: Int) -> Int external(g);`.
 * VSL programs will use the function name `f`, but outside of that it's
 * referred to by its alias `g`.
 */
class ExtFuncNode : public FuncInterfaceNode
{
public:
    /**
     * Creates an ExtFuncNode.
     *
     * @param location Where this FunctionNode was found in the source.
     * @param access Access specifier.
     * @param name The name of the function.
     * @param params The function's parameters.
     * @param returnType The type that the function returns.
     * @param alias What this function's actual name is outside of VSL.
     */
    ExtFuncNode(Location location, Access access, llvm::StringRef name,
        std::vector<ParamNode*> params, const Type* returnType,
        llvm::StringRef alias);
    virtual ~ExtFuncNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    llvm::StringRef getAlias() const;

private:
    /** The name this function is aliased to. */
    llvm::StringRef alias;
};

/**
 * Represents a function parameter, e.g.\ `x: Int`.
 */
class ParamNode : public Node
{
public:
    /**
     * Creates a ParamNode.
     *
     * @param location Where this ParamNode was found in the source.
     * @param name The name of the parameter.
     * @param type The type of the parameter.
     */
    ParamNode(Location location, llvm::StringRef name, const Type* type);
    virtual ~ParamNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    llvm::StringRef getName() const;
    const Type* getType() const;

private:
    /** The name of the parameter. */
    llvm::StringRef name;
    /** The type of the parameter. */
    const Type* type;
};

/**
 * Represents a variable declaration, e.g.\ `var x: Int = 5;`.
 */
class VariableNode : public DeclNode
{
public:
    /**
     * Creates a VariableNode.
     *
     * @param location Where this VariableNode was found in the source.
     * @param access Access specifier.
     * @param name The name of the variable.
     * @param type The type of the variable.
     * @param init The variable's initial value. Can be null.
     * @param constness If this variable is const or not (TODO).
     */
    VariableNode(Location location, Access access, llvm::StringRef name,
        const Type* type, ExprNode* init, bool constness);
    virtual ~VariableNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    llvm::StringRef getName() const;
    const Type* getType() const;
    bool hasInit() const;
    ExprNode& getInit() const;
    bool isConst() const;

protected:
    /**
     * Used by subclasses.
     */
    VariableNode(Node::Kind kind, Location location, Access access,
        llvm::StringRef name, const Type* type, ExprNode* init, bool constness);

private:
    /** The name of the variable. */
    llvm::StringRef name;
    /** The type of the variable. */
    const Type* type;
    /** The variable's initial value. */
    ExprNode* init;
    /** If this variable is const or not. */
    bool constness;
};

/**
 * Represents a class definition. An example of this would be:
 *
 *     public class A
 *     {
 *         public var x: Int; // FieldNode
 *         public init(x: Int) // CtorNode
 *         {
 *             self.x = x;
 *         }
 *         public func f(y: Int) -> Int // MethodNode
 *         {
 *             return x * y;
 *         }
 *     }
 */
class ClassNode : public DeclNode
{
public:
    /**
     * Represents a class member.
     */
    class Member
    {
    public:
        /**
         * Creates a Member object.
         *
         * @param parent The ClassNode this Member belongs to.
         */
        Member(ClassNode& parent);
        ClassNode& getParent() const;

    private:
        /** The ClassNode this Member belongs to. */
        ClassNode& parent;
    };

    /**
     * Creates a ClassNode. Use some of the other methods to add class members.
     *
     * @param location Where this ClassNode was found in the source.
     * @param access Access specifier.
     * @param name Name of the class.
     * @param namedType External type of the class. Should have classType as its
     * underlying type.
     * @param classType Class type equivalent.
     */
    ClassNode(Location location, Access access, llvm::StringRef name,
        const NamedType* type, ClassType* classType);
    virtual ~ClassNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    llvm::StringRef getName() const;
    const NamedType* getType() const;
    const ClassType* getClassType() const;
    llvm::ArrayRef<FieldNode*> getFields() const;
    size_t getNumFields() const;
    FieldNode& getField(size_t i) const;
    bool hasCtor() const;
    CtorNode& getCtor() const;
    llvm::ArrayRef<MethodNode*> getMethods() const;
    /**
     * Adds a field. This method fails and returns true if a field with the same
     * name already exists.
     *
     * @param field Field to add.
     *
     * @returns True if a duplicate field already exists, false otherwise.
     */
    bool addField(FieldNode& field);
    void setCtor(CtorNode& ctor);
    void addMethod(MethodNode& method);

private:
    /** Name of the class. */
    llvm::StringRef name;
    /** Type of the class. */
    const NamedType* type;
    /** Class type equivalent. */
    ClassType* classType;
    /** List of fields. */
    std::vector<FieldNode*> fields;
    /** Class constructor. Can be null. */
    CtorNode* ctor;
    /** List of instance methods. */
    std::vector<MethodNode*> methods;
};

/**
 * Represents a field of a class.
 */
class FieldNode : public VariableNode, public ClassNode::Member
{
public:
    /**
     * Creates a VariableNode.
     *
     * @param location Where this FieldNode was found in the source.
     * @param access Access specifier.
     * @param name Name of the field.
     * @param type Type of the field.
     * @param init The field's initial value. Can be null.
     * @param constness If this field is const or not (TODO).
     * @param parent The ClassNode this FieldNode belongs to.
     */
    FieldNode(Location location, Access access, llvm::StringRef name,
        const Type* type, ExprNode* init, bool constness, ClassNode& parent);
    virtual ~FieldNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
};

/**
 * Represents a method of a class.
 */
class MethodNode : public FunctionNode, public ClassNode::Member
{
public:
    /**
     * Creates a MethodNode.
     *
     * @param location Where this MethodNode was found in the source.
     * @param access Access specifier.
     * @param name The name of the method.
     * @param params The function's parameters.
     * @param returnType The type that the method returns.
     * @param body Body of the method.
     * @param parent The ClassNode this MethodNode belongs to.
     */
    MethodNode(Location location, Access access, llvm::StringRef name,
        std::vector<ParamNode*> params, const Type* returnType, BlockNode& body,
        ClassNode& parent);
    virtual ~MethodNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
};

/**
 * Represents a class constructor.
 */
class CtorNode : public FunctionNode, public ClassNode::Member
{
public:
    // TODO: could create UnresolvedType when parsing ClassNode and use that as
    //       the return type
    /**
     * Creates a CtorNode. The return type will be null until the class type is
     * resolved.
     *
     * @param location Where this MethodNode was found in the source.
     * @param access Access specifier.
     * @param params The function's parameters.
     * @param body Body of the method.
     * @param parent The ClassNode this CtorNode belongs to.
     */
    CtorNode(Location location, Access access, std::vector<ParamNode*> params,
        BlockNode& body, ClassNode& parent);
    virtual ~CtorNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
};

/**
 * Represents a block of code, e.g.\ `{ ... }`.
 */
class BlockNode : public Node
{
public:
    /**
     * Creates a BlockNode.
     *
     * @param location Where this BlockNode was found in the source.
     * @param statements The statements inside the block.
     */
    BlockNode(Location location, std::vector<Node*> statements);
    virtual ~BlockNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    llvm::ArrayRef<Node*> getStatements() const;

private:
    /** The statements inside the block. */
    std::vector<Node*> statements;
};

/**
 * Represents an empty statement, e.g.\ `;`.
 */
class EmptyNode : public Node
{
public:
    /**
     * Creates an EmptyNode.
     *
     * @param location Where this EmptyNode was found in the source.
     */
    EmptyNode(Location location);
    virtual ~EmptyNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
};

/**
 * Represents an if/else statement, e.g.\ `if (x) { ... } else { ... }`.
 */
class IfNode : public Node
{
public:
    /**
     * Creates an IfNode.
     *
     * @param location Where this IfNode was found in the source.
     * @param condition The condition to test.
     * @param thenCase The code to run if the condition is true.
     * @param elseCase The code to run if the condition is false. Can be null.
     */
    IfNode(Location location, ExprNode& condition, Node& thenCase,
        Node* elseCase);
    virtual ~IfNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    ExprNode& getCondition() const;
    Node& getThen() const;
    bool hasElse() const;
    Node& getElse() const;

private:
    /** The condition to test. */
    ExprNode& condition;
    /** The code to run if the condition is true. */
    Node& thenCase;
    /** The code to run if the condition is false. Can be null. */
    Node* elseCase;
};

/**
 * Represents a return statement.
 */
class ReturnNode : public Node
{
public:
    /**
     * Creates a ReturnNode.
     *
     * @param location Where this ReturnNode was found in the source.
     * @param value The value to return. Can be null.
     */
    ReturnNode(Location location, ExprNode* value);
    virtual ~ReturnNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    bool hasValue() const;
    ExprNode& getValue() const;

private:
    /** The value to return. */
    ExprNode* value;
};

/**
 * Base class for expressions.
 */
class ExprNode : public Node
{
public:
    /**
     * Creates an ExprNode.
     *
     * @param kind The kind of ExprNode this is.
     * @param location where this ExprNode was found in the source.
     */
    ExprNode(Node::Kind kind, Location location);
    virtual bool isExpr() const override;
};

/**
 * Represents an identifier, e.g.\ `x`.
 */
class IdentNode : public ExprNode
{
public:
    /**
     * Creates an IdentNode.
     *
     * @param location where this IdentNode was found in the source.
     * @param name The name of the identifier.
     */
    IdentNode(Location location, llvm::StringRef name);
    virtual ~IdentNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    llvm::StringRef getName() const;

private:
    /** The name of the identifier. */
    llvm::StringRef name;
};

/**
 * Represents an (integer) literal, e.g.\ `1337`.
 */
class LiteralNode : public ExprNode
{
public:
    /**
     * Creates an LiteralNode.
     *
     * @param location Where this LiteralNode was found in the source.
     * @param value The value of the number.
     */
    LiteralNode(Location location, llvm::APInt value);
    virtual ~LiteralNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    llvm::APInt getValue() const;

private:
    /** The value of the number. */
    llvm::APInt value;
};

/**
 * Represents a unary expression, e.g.\ `-1`.
 */
class UnaryNode : public ExprNode
{
public:
    /**
     * Creates a UnaryNode.
     *
     * @param location Where this UnaryNode was found in the source.
     * @param op The operator of the expression.
     * @param expr The expression to apply the operator to.
     */
    UnaryNode(Location location, UnaryKind op, ExprNode& expr);
    virtual ~UnaryNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    UnaryKind getOp() const;
    const char* getOpSymbol() const;
    ExprNode& getExpr() const;

private:
    /** The operator of the expression. */
    UnaryKind op;
    /** The expression to apply the operator to. */
    ExprNode& expr;
};

/**
 * Represents a binary expression, e.g.\ `1+1`.
 */
class BinaryNode : public ExprNode
{
public:
    /**
     * Creates a BinaryNode.
     *
     * @param location Where this BinaryNode was found in the source.
     * @param op The operator of the expression.
     * @param left The left hand side of the expression.
     * @param right The right hand side of the expression.
     */
    BinaryNode(Location location, BinaryKind op, ExprNode& left,
        ExprNode& right);
    virtual ~BinaryNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    BinaryKind getOp() const;
    const char* getOpSymbol() const;
    ExprNode& getLhs() const;
    ExprNode& getRhs() const;

private:
    /** The operator of the expression. */
    BinaryKind op;
    /** The left hand side of the expression. */
    ExprNode& left;
    /** The right hand side of the expression. */
    ExprNode& right;
};

/**
 * Represents a ternary expression, e.g.\ `c ? x : y`.
 */
class TernaryNode : public ExprNode
{
public:
    /**
     * Creates a TernaryNode.
     *
     * @param location Where this TernaryNode was found in the source.
     * @param condition The condition to test for.
     * @param thenCase The expression when the condition is true.
     * @param elseCase The expression when the condition is false.
     */
    TernaryNode(Location location, ExprNode& condition, ExprNode& thenCase,
        ExprNode& elseCase);
    virtual ~TernaryNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    ExprNode& getCondition() const;
    ExprNode& getThen() const;
    ExprNode& getElse() const;

private:
    /** The condition to test for. */
    ExprNode& condition;
    /** The expression when the condition is true. */
    ExprNode& thenCase;
    /** The expression when the condition is false. */
    ExprNode& elseCase;
};

/**
 * Represents a function call, e.g.\ `f(x: 1)`.
 */
class CallNode : public ExprNode
{
public:
    /**
     * Creates a CallNode.
     *
     * @param location Where this CallNode was found in the source.
     * @param callee The function to call.
     * @param args The arguments to pass to the callee.
     */
    CallNode(Location location, ExprNode& callee, std::vector<ArgNode*> args);
    virtual ~CallNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    ExprNode& getCallee() const;
    llvm::ArrayRef<ArgNode*> getArgs() const;
    size_t getNumArgs() const;
    ArgNode& getArg(size_t i) const;

protected:
    /**
     * Creates a CallNode. This allows subclasses to have a different
     * Node::Kind.
     *
     * @param kind The kind of CallNode this is.
     * @param location Where this CallNode was found in the source.
     * @param callee The function to call.
     * @param args The arguments to pass to the callee.
     */
    CallNode(Node::Kind kind, Location location, ExprNode& callee,
        std::vector<ArgNode*> args);

private:
    /** The function to call. */
    ExprNode& callee;
    /** The arguments to pass to the callee. */
    std::vector<ArgNode*> args;
};

/**
 * Represents a function call argument, e.g.\ `x: 1`.
 */
class ArgNode : public Node
{
public:
    /**
     * Creates an ArgNode.
     *
     * @param location Where this ArgNode was found in the source.
     * @param name The name of the argument. Used for named parameters (TODO).
     * @param value The value of the argument.
     */
    ArgNode(Location location, llvm::StringRef name, ExprNode& value);
    virtual ~ArgNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    llvm::StringRef getName() const;
    ExprNode& getValue() const;

private:
    /** The name of the argument. */
    llvm::StringRef name;
    /** The value of the argument. */
    ExprNode& value;
};

/**
 * Represents a field access, e.g.\ `object.field`.
 */
class FieldAccessNode : public ExprNode
{
public:
    /**
     * Creates a FieldAccessNode.
     *
     * @param location Where this FieldAccessNode was found in the source.
     * @param object Object to access a field of.
     * @param field Name of the field to access.
     */
    FieldAccessNode(Location location, ExprNode& object, llvm::StringRef field);
    virtual ~FieldAccessNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    ExprNode& getObject() const;
    llvm::StringRef getField() const;

private:
    /** Object to access a field of. */
    ExprNode& object;
    /** Name of the field to access. */
    llvm::StringRef field;
};

/**
 * Represents a method call, e.g.\ `callee.method(arg: 10)`.
 */
class MethodCallNode : public CallNode
{
public:
    /**
     * Creates a MethodCallNode.
     *
     * @param location Where this MethodCallNode was found in the source.
     * @param callee Object to access a method of.
     * @param method Name of the method.
     * @param args Arguments to pass to the callee+method.
     */
    MethodCallNode(Location location, ExprNode& callee, llvm::StringRef method,
        std::vector<ArgNode*> args);
    virtual ~MethodCallNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    llvm::StringRef getMethod() const;

private:
    /** Name of the method. */
    llvm::StringRef method;
};

/**
 * Represents the `self` keyword.
 */
class SelfNode : public ExprNode
{
public:
    /**
     * Creates a SelfNode.
     *
     * @param location Where this SelfNode was found in the source.
     */
    SelfNode(Location location);
    virtual ~SelfNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
};

#endif // NODE_HPP
