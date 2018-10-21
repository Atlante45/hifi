//
// Sample file to demonstrate various clang-format messages
//

// SortIncludes: true
// IncludeBlacks: Preserve
#include <stdio.h>
#include <vector>

#include <assert.h>
#include <string>

#define BIT_MASK 0xDEADBEAF

// AlignEscapedNewlines: Right
#define MULTILINE_DEF(a, b)                                                                                                    \
    if ((a) > 2) {                                                                                                             \
        auto temp = (b) / 2;                                                                                                   \
        (b) += 10;                                                                                                             \
        someFunctionCall((a), (b));                                                                                            \
    }

// NamespaceIndentation: None
namespace foo {
// Contents of namespace are not indented.
int foo;

namespace bar {
int bar;

namespace {
int anony;
} // namespace

// FixNamespaceComments: true
} // namespace bar

class ALongTypeNameeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee {};
class AnotherLongTypeNameeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee {};

// BreakInheritanceList: AfterColon
class ShortList : public int, public float {};
class Example :
    public ALongTypeNameeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee,
    public AnotherLongTypeNameeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee {
    // AccessModiferOffset: -4
public:
    // BreakConstructorInitializers: AfterColon
    // ConstructorInitializerAllOnOneLineOrOnePerLine: true
    Example() : _aVariable(4), _bVariable(42) {}
    Example(int a) :
        _aVariable(a),
        _bVariable(42),
        _AReallyReallyLongVariableName(4),
        _AnotherReallyReallyLongVariableNameToTriggerWrapping(42) {
        printf("Hello ");
    }
    ~Example() {}

    // AllowShortFunctionsOnASingleLine: InlineOnly
    int getOneLineFunction() { return 0; }

private:
    int _aVariable;
    long _bVariable { 0 };
    short _cVarianble { 49 };
    long _AReallyReallyLongVariableName;
    long _AnotherReallyReallyLongVariableNameToTriggerWrapping;
};

// AllowShortFunctionsOnASingleLine: InlineOnly
int foo3() {
    return 42;
}

// AlwaysBreakTemplateDeclarations: true
template<typename T>
T myAdd(T x, T y) {
    return x + y;
}

// AlwaysBreakAfterReturnType: None
// BinPackParameters: true
int Example::manyVariableFunction(unsigned long long argWithLongNameeeeeeeeeeeeeeee, char arg2,
                                  unsigned long long argWithAnotherLongNameeeeeeeeeeeeeeeeee) {
    // AlignConsecutiveAssignments: false
    // AlignConsecutiveDeclarations: false
    int aaaa = 12;
    int b = 23;
    int ccc = 23;
    // PointerAlignment: Left
    const char* some_pointer = "Hello";

    // SpacesInAngles: false
    std::vector<std::pair<std::string, int>> list;

    // SpacesInCStyleCastParentheses: false
    char* some_nonconst_pointer = (char*)some_pointer;

    // Multi-line if
    // SpaceBeforeParens: ControlStatements
    if (argWithLongNameeeeeeeeeeeeeeee == 0) { // Comment: SpacesBeforeTrailingComments
        foo3();
        // Do something
    } else if (b % 7 = 3) {
    } // some weird trailing else comment that clang-format does not touch
    else {
    }

    int bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb, ccccccccccccccccccccccccccccccccccccccccccccccccccccc;
    int aaaaaaaaaaaaaaaaaaaaaaaaaaaa = bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb +
                                       ccccccccccccccccccccccccccccccccccccccccccccccccccccc;

    // AlignOperands
    int dddddddddddddddddddddddddddd = aaaaaaaaaaaaaaaaaaaaaaaaaaaa * 7 + 4 % 235124 > 275645
                                           ? bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb + 897234
                                           : ccccccccccccccccccccccccccccccccccccccccccccccccccccc % 1293402;

    // AllowShortIfStatementsOnASingleLine: false
    // Put statements on separate lines for short ifs
    if (arg2 == 'a')
        arg2 = 'b';

    // AllowShortBlocksOnASingleLine: false
    if (b) {
        return 3;
    }

    // AllowShortLoopsOnASingleLine: false
    while (b < 5)
        b++;

    // BreakBeforeBinaryOperators: None
    if (b > 5 || b % 42 || ccccccccccccccccccccccccccccccccccccccccccccccccccccc % 873 || aaaa * 12312 % 23485 != 9873 ||
        some_pointer != 0) {
        printf("Huh!\n");
    }

    // AlignAfterOpenBracket: false
    // BinPackParameters: true
    printf("A short function call %s %s %d - %ld\n", "", "tgz", 4, ULONG_MAX);
    printf("A long function call %s %s %d - %ld\n", "a long string!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
           "another long string!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!", 4, ULONG_MAX);
    printf("Thing1 %s\n", "Thing2");

    // No spaces between parens and args
    printf("%c\n", arg2);

    // A switch statement
    switch (arg2) {
            // AllowShortCaseLabelsOnASingleLine: false
            // IndentCaseLabels: true
        case 'a':
            return 2;
        case 'y':
        case 'z':
            // Do something here
            break;
        default:
            // The default`
            break;
    }

    do {
        // Do a loop here
    } while (0);

    std::cout << "Congratulations, you sorted the list.\n"
              << "You needed " << score << " reversals." << std::endl;

    return 1;
}

} // namespace foo
