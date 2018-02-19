#ifndef PARSER_HPP
#define PARSER_HPP

#include "ast/node.hpp"
#include "ast/vslContext.hpp"
#include "llvm/ADT/ArrayRef.h"
#include <memory>
#include <type_traits>

/**
 * Base class for parsers.
 */
class Parser
{
public:
    Parser(VSLContext& vslCtx);
    virtual ~Parser() = 0;
    /**
     * Parses the program. The `DeclNode*`s returned are owned by the VSLContext
     * provided by this class' constructor.
     *
     * @returns The AST of the program, wrapped in an ArrayRef of global
     * declarations.
     */
    virtual llvm::ArrayRef<DeclNode*> parse() = 0;

protected:
    /**
     * Creates a Node.
     *
     * @tparam NodeT The Node-derived type to instantiate.
     * @tparam Args NodeT's constructor arguments.
     *
     * @param args NodeT's constructor arguments.
     *
     * @returns A pointer to a newly created NodeT.
     */
    template<typename NodeT, typename... Args>
    typename std::enable_if<std::is_base_of<Node, NodeT>::value, NodeT*>::type
    makeNode(Args&&... args)
    {
        auto node = std::make_unique<NodeT>(std::forward<Args>(args)...);
        NodeT* nodePtr = node.get();
        vslCtx.addNode(std::move(node));
        return nodePtr;
    }

    /** Reference to the VSLContext. */
    VSLContext& vslCtx;
};

#endif // PARSER_HPP
