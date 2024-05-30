=== key idea

t_s := F_s                              (Function)
     | v                                (Value)
     | [ t_s, ..., t_s ]                (Array)
     | { str: t_s, ..., str: t_s }      (Object)
     | x                                (Var)
     | lambda x . t_s                   (Lambda)
     | t_s ( t_s, ..., t_s )            (Application)

t := F
   | v
   | [ t, ..., t ]                (Array)
   | { str: t, ..., str: t }      (Object)
   | x
   | lambda x . t
   | t ( t, ..., t )

compile: t_s |-> t


=== Issue 1: variable shadowing

in t_s:
```
struct Variable {
    std::string name;
};
```

in t:
```
struct Variable {
    size_t id;
};
```


=== Issue 2: function matching

in t:
```
Function:
    type: Int -> Int -> Int
    alias: Add                (<<<<< Match)
```

in t_s:
```
Function:
    name: Add                 (<<<<< Match)
```

```
struct FunctionType {
    Type result;
    std::vector<Type> arguments;
};
```

tau ::= F | tau -> tau | Any
      | Object[tau]
      | Array[tau]
      | Schema{str: tau, ..., str: tau}

F ::= Int | Double | String | Bool | None


How to match functions?

- add custom inference rules

```
p : Int   q : Int
-----------------
Add p q : Int              AND  Add_s -> Func { id=<add_int> }

p : Double   q : Double
-----------------------
Add p q : Double           AND  Add_s -> Func { id=<add_double> }
```

```
AsInt(Add($x, $y))
```

