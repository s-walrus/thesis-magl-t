# How to execute a MAGL-T expression?



```py
def execute(t: Term) -> Value:
    if t is Value:
        return t

    args = [execute(arg) for arg in t.args]

    return t.call(args)
```


```

for arg in t.args
    execute(arg)

```


Min jumps
---------
Term is a virtual class with `virtual T Execute(...);`

No arg type checks
------------------
```cpp
using ArgHodler = void;

struct ArgsContainer {
    ArgHolder* operator[](size_t i);
};

struct Term {
    // TODO: Consider void -> Error
    void Execute(const ArgsContainer* args, ArgHolder* to) = 0;
};

void SumInt(const ArgsContainer* args, ArgHolder* to) {
    const int64_t lhs = *reinterpret_cast<int64_t*>(args[0]);
    const int64_t rhs = *reinterpret_cast<int64_t*>(args[1]);
    *reinterpret_cast<int64_t*>(ArgHolder) = lhs + rhs;
}
```


Min cache misses
----------------
```cpp
// For the MVP:
using ArgsContainer = std::array<ArgHolder, kMaxArgsSize>;



// For later optimizations:

using ArgsContainer = byte[64]  // ???

size_t arg_size = max(t->GetArgsSize() for t in all_terms);
using ArgsContainer = byte[args_size]  // ???
```


Common
------


```cpp
struct Term {
    Term() = 0;

    void Execute(const ArgsContainer* args, ArgHolder* to) = 0;

    const size_t args_count;
    const std::array<std::unique_ptr<Term>, kMaxArgs> args;
}
```
