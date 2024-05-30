LIBRARY()

PEERDIR(
    contrib/restricted/boost/variant
    contrib/restricted/boost/stacktrace
)

ADDINCL(
    GLOBAL src
)

SRCDIR(
    src
)

SRCS(
    executer/evaluate.cpp
    executer/expression.cpp
    executer/term.cpp
    functions/include.cpp
    functions/library/add.cpp
    functions/library/ducttape-get-var.cpp
    functions/library/ducttape-var-map-at.cpp
    functions/library/get-value.cpp
    functions/library/insert.cpp
    functions/library/append.cpp
    functions/library/map.cpp
    functions/library/multiply.cpp
    functions/type.cpp
    functions/utils/type-equivalent.cpp
    functions/utils/value-type.cpp
    parser/parser.cpp
    parser/syntax/functions.cpp
    parser/syntax/operator-presedence.cpp
    parser/syntax/parser.cpp
    parser/syntax/syntax-tree.cpp
    parser/syntax/validation.cpp
    parser/terms/compiler.cpp
    parser/terms/inference/inference.cpp
    parser/terms/inference/unification.cpp
    parser/terms/semantic-graph.cpp
    parser/terms/terms.cpp
    parser/tokenizer/tokenizer.cpp
    parser/tokenizer/tokens.cpp
    parser/utils/overloaded.cpp
    parser/utils/stream.cpp
    value/value.cpp
)

END()
