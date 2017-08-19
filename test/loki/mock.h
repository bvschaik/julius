#define INIT_MOCK(f) reset_##f();
#define SETUP(f) f();
#define INIT_MOCKS(block) \
void setup_mocks()\
{\
    block\
}

#define NO_MOCKS() void setup_mocks() {}

#define _VMOCK_STRUCTS(f, vars)\
struct call_##f {\
    vars\
    struct call_##f *next;\
};\
struct {\
    struct call_##f *actual;\
} calls_##f = {0};

#define _MOCK_DYN_STRUCT(f, r, types)\
struct call_dynamic_##f {\
    int (*func)types;\
    r then_return;\
    struct call_dynamic_##f *next;\
};

#define _MOCK_STRUCTS(f, r, vars)\
struct call_##f {\
    vars\
    r then_return;\
    struct call_##f *next;\
};\
struct {\
    struct call_##f *actual;\
    struct call_##f *expected;\
    struct call_dynamic_##f *dynamic;\
} calls_##f = {0, 0, 0};

#define _MOCK_ADD_CALL(f, field, vars)\
    struct call_##f *call = (struct call_##f*) malloc(sizeof(struct call_##f));\
    vars;\
    call->next = calls_##f.field;\
    calls_##f.field = call;

#define _MOCK_ADD_DYN_CALL(f)\
    struct call_dynamic_##f *call = (struct call_dynamic_##f*) malloc(sizeof(struct call_dynamic_##f));\
    call->func = func;\
    call->next = calls_##f.dynamic;\
    calls_##f.dynamic = call;

#define _MOCK_WALK_CALLS(f, field, onmatch, eq)\
    struct call_##f *walk = calls_##f.field;\
    while (walk) {\
        if (eq) {\
            onmatch;\
        }\
        walk = walk->next;\
    }

#define _MOCK_WALK_DYN(r, f, fields)\
    struct call_dynamic_##f *dyn = calls_##f.dynamic;\
    while (dyn) {\
        if (dyn->func fields) {\
            return dyn->then_return;\
        }\
        dyn = dyn->next;\
    }

#define _MOCK_FAIL(f)\
    testsuite_record_fail(&testsuite, "FAIL: expected call to '"#f"'\n");\

    
#define _MOCK_DELETE_CALLS(f, type, field) {\
    struct type *call = calls_##f.field;\
    while (call) {\
        struct type *next = call->next;\
        free(call);\
        call = next;\
    }\
    calls_##f.field = NULL;\
}

#define _MOCK_RESET(f)\
void reset_##f() {\
    _MOCK_DELETE_CALLS(f, call_##f, actual)\
    _MOCK_DELETE_CALLS(f, call_##f, expected)\
    _MOCK_DELETE_CALLS(f, call_dynamic_##f, dynamic)\
}

#define _VMOCK_RESET(f)\
void reset_##f() {\
    struct call_##f *call = calls_##f.actual;\
    while (call) {\
        struct call_##f *next = call->next;\
        free(call);\
        call = next;\
    }\
    calls_##f.actual = NULL;\
}


#define CREATE_VMOCK(f, args, defs, assigns, checks, args_walk) \
_VMOCK_STRUCTS(f, defs)\
void f args\
{\
    _MOCK_ADD_CALL(f, actual, assigns)\
}\
void verify_##f args\
{\
    _MOCK_WALK_CALLS(f, actual, return, checks)\
    _MOCK_FAIL(f)\
}\
void verify_##f##_dynamic(int (*func) args)\
{\
    _MOCK_WALK_CALLS(f, actual, return, func args_walk)\
    _MOCK_FAIL(f)\
}\
void verify_##f##_times(int times)\
{\
    int total = 0;\
    _MOCK_WALK_CALLS(f, actual, total++, 1)\
    if (total != times) {\
        testsuite_record_fail(&testsuite, "FAIL: expected call %d calls to '"#f"', recorded %d\n", times, total);\
    }\
}\
_VMOCK_RESET(f)


#define CREATE_VMOCK0(f)\
CREATE_VMOCK(f,\
    (),\
    ,\
    ,\
    1,\
    ()\
)

#define CREATE_VMOCK1(f, t1)\
CREATE_VMOCK(f,\
    (t1 a1),\
    t1 a1;,\
    call->a1 = a1,\
    walk->a1 == a1,\
    (walk->a1)\
)

#define CREATE_VMOCK2(f, t1, t2)\
CREATE_VMOCK(f,\
    (t1 a1, t2 a2),\
    t1 a1; t2 a2;,\
    call->a1 = a1; call->a2 = a2,\
    walk->a1 == a1 && walk->a2 == a2,\
    (walk->a1, walk->a2)\
)

#define CREATE_VMOCK3(f, t1, t2, t3)\
CREATE_VMOCK(f,\
    (t1 a1, t2 a2, t3 a3),\
    t1 a1; t2 a2; t3 a3;,\
    call->a1 = a1; call->a2 = a2; call->a3 = a3,\
    walk->a1 == a1 && walk->a2 == a2 && walk->a3 == a3,\
    (walk->a1, walk->a2, walk->a3)\
)

#define CREATE_VMOCK4(f, t1, t2, t3, t4)\
CREATE_VMOCK(f,\
    (t1 a1, t2 a2, t3 a3, t4 a4),\
    t1 a1; t2 a2; t3 a3; t4 a4;,\
    call->a1 = a1; call->a2 = a2; call->a3 = a3; call->a4 = a4,\
    walk->a1 == a1 && walk->a2 == a2 && walk->a3 == a3 && walk->a4 == a4,\
    (walk->a1, walk->a2, walk->a3, walk->a4)\
)

#define CREATE_VMOCK5(f, t1, t2, t3, t4, t5)\
CREATE_VMOCK(f,\
    (t1 a1, t2 a2, t3 a3, t4 a4, t5 a5),\
    t1 a1; t2 a2; t3 a3; t4 a4; t5 a5;,\
    call->a1 = a1; call->a2 = a2; call->a3 = a3; call->a4 = a4; call->a5 = a5,\
    walk->a1 == a1 && walk->a2 == a2 && walk->a3 == a3 && walk->a4 == a4 && walk->a5 == a5,\
    (walk->a1, walk->a2, walk->a3, walk->a4, walk->a5)\
)


#define CREATE_MOCK(r, f, args, defs, assigns, checks, args_only, args_walk) \
_MOCK_DYN_STRUCT(f, r, args)\
_MOCK_STRUCTS(f, r, defs)\
r f args\
{\
    _MOCK_ADD_CALL(f, actual, assigns)\
    _MOCK_WALK_CALLS(f, expected, return walk->then_return, checks)\
    _MOCK_WALK_DYN(r, f, args_only);\
    return 0;\
}\
struct call_##f *when_##f args\
{\
    _MOCK_ADD_CALL(f, expected, assigns)\
    return call;\
}\
struct call_dynamic_##f *when_##f##_dynamic(int (*func) args)\
{\
    _MOCK_ADD_DYN_CALL(f)\
    return call;\
}\
int any_##f args\
{\
    return 1;\
}\
void verify_##f args\
{\
    _MOCK_WALK_CALLS(f, actual, return, checks)\
    _MOCK_FAIL(f)\
}\
void verify_##f##_dynamic(int (*func) args)\
{\
    _MOCK_WALK_CALLS(f, actual, return, func args_walk)\
    _MOCK_FAIL(f)\
}\
void verify_##f##_times(int times)\
{\
    int total = 0;\
    _MOCK_WALK_CALLS(f, actual, total++, 1)\
    if (total != times) {\
        testsuite_record_fail(&testsuite, "FAIL: expected call %d calls to '"#f"', recorded %d\n", times, total);\
    }\
}\
_MOCK_RESET(f)


#define CREATE_MOCK0(r, f)\
CREATE_MOCK(r, f,\
    (),\
    ,\
    ,\
    1,\
    (),\
    ()\
)

#define CREATE_MOCK1(r, f, t1)\
CREATE_MOCK(r, f,\
    (t1 a1),\
    t1 a1;,\
    call->a1 = a1,\
    walk->a1 == a1,\
    (a1),\
    (walk->a1)\
)

#define CREATE_MOCK2(r, f, t1, t2)\
CREATE_MOCK(r, f,\
    (t1 a1, t2 a2),\
    t1 a1; t2 a2;,\
    call->a1 = a1; call->a2 = a2,\
    walk->a1 == a1 && walk->a2 == a2,\
    (a1, a2),\
    (walk->a1, walk->a2)\
)

#define CREATE_MOCK3(r, f, t1, t2, t3)\
CREATE_MOCK(r, f,\
    (t1 a1, t2 a2, t3 a3),\
    t1 a1; t2 a2; t3 a3;,\
    call->a1 = a1; call->a2 = a2; call->a3 = a3,\
    walk->a1 == a1 && walk->a2 == a2 && walk->a3 == a3,\
    (a1, a2, a3),\
    (walk->a1, walk->a2, walk->a3)\
)

#define CREATE_MOCK4(r, f, t1, t2, t3, t4)\
CREATE_MOCK(r, f,\
    (t1 a1, t2 a2, t3 a3, t4 a4),\
    t1 a1; t2 a2; t3 a3; t4 a4;,\
    call->a1 = a1; call->a2 = a2; call->a3 = a3; call->a4 = a4,\
    walk->a1 == a1 && walk->a2 == a2 && walk->a3 == a3 && walk->a4 == a4,\
    (a1, a2, a3, a4),\
    (walk->a1, walk->a2, walk->a3, walk->a4)\
)

#define CREATE_MOCK5(r, f, t1, t2, t3, t4, t5)\
CREATE_MOCK(r, f,\
    (t1 a1, t2 a2, t3 a3, t4 a4, t5 a5),\
    t1 a1; t2 a2; t3 a3; t4 a4; t5 a5;,\
    call->a1 = a1; call->a2 = a2; call->a3 = a3; call->a4 = a4; call->a5 = a5,\
    walk->a1 == a1 && walk->a2 == a2 && walk->a3 == a3 && walk->a4 == a4 && walk->a5 == a5,\
    (a1, a2, a3, a4, a5),\
    (walk->a1, walk->a2, walk->a3, walk->a4, walk->a5)\
)

