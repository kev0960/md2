#include "generators/asm_syntax_highlighter.h"
#include "generators/cpp_syntax_highlighter.h"
#include "generators/generator_context.h"
#include "generators/objdump_highlighter.h"
#include "generators/py_syntax_highlighter.h"
#include "gtest/gtest.h"
#include "logger.h"
#include "metadata_repo.h"

namespace md2 {
namespace {

std::string TokenTypeToString(SyntaxTokenType type) {
  switch (type) {
    case KEYWORD:
      return "KEYWORD";
    case TYPE_KEYWORD:
      return "TYPE_KEYWORD";
    case IDENTIFIER:
      return "IDENTIFIER";
    case NUMERIC_LITERAL:
      return "NUMERIC_LITERAL";
    case STRING_LITERAL:
      return "STRING_LITERAL";
    case BRACKET:
      return "BRACKET";
    case PARENTHESES:
      return "PARENTHESES";
    case BRACE:
      return "BRACE";
    case PUNCTUATION:
      return "PUNCTUATION";
    case OPERATOR:
      return "OPERATOR";
    case COMMENT:
      return "COMMENT";
    case MACRO_HEAD:
      return "MACRO_HEAD";
    case MACRO_BODY:
      return "MACRO_BODY";
    case WHITESPACE:
      return "WHITESPACE";
    case FUNCTION:
      return "FUNCTION";
    case BUILT_IN:
      return "BUILT_IN";
    case MAGIC_FUNCTION:
      return "MAGIC_FUNCTION";
    case REGISTER:
      return "REGISTER";
    case LABEL:
      return "LABEL";
    case DIRECTIVE:
      return "DIRECTIVE";
    case INSTRUCTION:
      return "INSTRUCTION";
    case FUNCTION_SECTION:
      return "FUNCTION_SECTION";
    case NONE:
      return "NONE";
  }
  return "";
}
}  // namespace

template <typename Highlighter>
class SyntaxHighlighterTester {
 public:
  SyntaxHighlighterTester() : fake_context_(repo_, "") {}

  void CheckSyntaxTokens(std::vector<SyntaxToken> expected) {
    const std::vector<SyntaxToken>& actual =
        static_cast<Highlighter*>(this)->GetTokenList();

    EXPECT_EQ(actual.size(), expected.size());
    for (size_t i = 0; i < std::min(actual.size(), expected.size()); i++) {
      if (i < actual.size() && i < actual.size()) {
        EXPECT_EQ(TokenTypeToString(actual[i].token_type),
                  TokenTypeToString(expected[i].token_type));
        EXPECT_EQ(actual[i].token_start, expected[i].token_start);
        EXPECT_EQ(actual[i].token_end, expected[i].token_end);
      }
    }
  }

  void PrintTokens() {
    const std::vector<SyntaxToken>& token_list =
        static_cast<Highlighter*>(this)->GetTokenList();

    for (size_t i = 0; i < token_list.size(); i++) {
      LOG(0) << TokenTypeToString(token_list[i].token_type);
      LOG(0) << " [" << token_list[i].token_start << " , "
             << token_list[i].token_end << "]";
    }
  }

  GeneratorContext fake_context_;
  MetadataRepo repo_;
};

class MockSyntaxHighlighter
    : public CppSyntaxHighlighter,
      public SyntaxHighlighterTester<MockSyntaxHighlighter> {
 public:
  MockSyntaxHighlighter(std::string_view content)
      : CppSyntaxHighlighter(content, "cpp") {}
};

class MockPySyntaxHighlighter
    : public PySyntaxHighlighter,
      public SyntaxHighlighterTester<MockPySyntaxHighlighter> {
 public:
  MockPySyntaxHighlighter(std::string_view content)
      : PySyntaxHighlighter(content, "py") {}
};

TEST(SyntaxHighlightTest, CppMacro) {
  MockSyntaxHighlighter syn("#include <iostream>");
  syn.ParseCode();
  syn.CheckSyntaxTokens(
      {{MACRO_HEAD, 0, 8}, {WHITESPACE, 8, 9}, {MACRO_BODY, 9, 19}});

  MockSyntaxHighlighter syn2("#define  SOME_FUNC(X, X) \\\nX*X\n");
  syn2.ParseCode();
  syn2.CheckSyntaxTokens({{MACRO_HEAD, 0, 7},
                          {WHITESPACE, 7, 9},
                          {MACRO_BODY, 9, 30},
                          {WHITESPACE, 30, 31}});
}

TEST(SyntaxHighlightTest, CppMacroComplex) {
  MockSyntaxHighlighter syn2("#ifdef AAAA\nhi;\n#endif");
  syn2.ParseCode();
  syn2.CheckSyntaxTokens({{MACRO_HEAD, 0, 6},
                          {WHITESPACE, 6, 7},
                          {MACRO_BODY, 7, 11},
                          {WHITESPACE, 11, 12},
                          {IDENTIFIER, 12, 14},
                          {PUNCTUATION, 14, 15},
                          {WHITESPACE, 15, 16},
                          {MACRO_HEAD, 16, 22}});
}

TEST(SyntaxHighlightTest, StringLiterals) {
  MockSyntaxHighlighter syn("'a';'b';");
  syn.ParseCode();
  syn.CheckSyntaxTokens({{STRING_LITERAL, 0, 3},
                         {PUNCTUATION, 3, 4},
                         {STRING_LITERAL, 4, 7},
                         {PUNCTUATION, 7, 8}});
}

TEST(SyntaxHighlightTest, StringLiterals2) {
  MockSyntaxHighlighter syn2(R"(string s = "hi \"asdf";)");
  syn2.ParseCode();
  syn2.CheckSyntaxTokens({{TYPE_KEYWORD, 0, 6},
                          {WHITESPACE, 6, 7},
                          {IDENTIFIER, 7, 8},
                          {WHITESPACE, 8, 9},
                          {OPERATOR, 9, 10},
                          {WHITESPACE, 10, 11},
                          {STRING_LITERAL, 11, 22},
                          {PUNCTUATION, 22, 23}});
}

TEST(SyntaxHighlightTest, StringLiterals3) {
  MockSyntaxHighlighter syn3(R"test(string s = R"(some"") \ \ a)";)test");
  syn3.ParseCode();
  syn3.CheckSyntaxTokens({{TYPE_KEYWORD, 0, 6},
                          {WHITESPACE, 6, 7},
                          {IDENTIFIER, 7, 8},
                          {WHITESPACE, 8, 9},
                          {OPERATOR, 9, 10},
                          {WHITESPACE, 10, 11},
                          {IDENTIFIER, 11, 12},
                          {STRING_LITERAL, 12, 29},
                          {PUNCTUATION, 29, 30}});

  MockSyntaxHighlighter syn4(
      R"test(string s = R"abc(some")")" \ \ a)abc";)test");
  syn4.ParseCode();
  syn4.CheckSyntaxTokens({{TYPE_KEYWORD, 0, 6},
                          {WHITESPACE, 6, 7},
                          {IDENTIFIER, 7, 8},
                          {WHITESPACE, 8, 9},
                          {OPERATOR, 9, 10},
                          {WHITESPACE, 10, 11},
                          {IDENTIFIER, 11, 12},
                          {STRING_LITERAL, 12, 37},
                          {PUNCTUATION, 37, 38}});
}

TEST(SyntaxHighlightTest, CppNumerals) {
  MockSyntaxHighlighter syn("int a = -123;");
  syn.ParseCode();
  syn.CheckSyntaxTokens({{TYPE_KEYWORD, 0, 3},
                         {WHITESPACE, 3, 4},
                         {IDENTIFIER, 4, 5},
                         {WHITESPACE, 5, 6},
                         {OPERATOR, 6, 7},
                         {WHITESPACE, 7, 8},
                         {OPERATOR, 8, 9},
                         {NUMERIC_LITERAL, 9, 12},
                         {PUNCTUATION, 12, 13}});

  MockSyntaxHighlighter syn2("float a = 1.2f;");
  syn2.ParseCode();
  syn2.CheckSyntaxTokens({{TYPE_KEYWORD, 0, 5},
                          {WHITESPACE, 5, 6},
                          {IDENTIFIER, 6, 7},
                          {WHITESPACE, 7, 8},
                          {OPERATOR, 8, 9},
                          {WHITESPACE, 9, 10},
                          {NUMERIC_LITERAL, 10, 14},
                          {PUNCTUATION, 14, 15}});

  MockSyntaxHighlighter syn3("float a = .1E4f;");
  syn3.ParseCode();
  syn3.CheckSyntaxTokens({{TYPE_KEYWORD, 0, 5},
                          {WHITESPACE, 5, 6},
                          {IDENTIFIER, 6, 7},
                          {WHITESPACE, 7, 8},
                          {OPERATOR, 8, 9},
                          {WHITESPACE, 9, 10},
                          {NUMERIC_LITERAL, 10, 15},
                          {PUNCTUATION, 15, 16}});

  MockSyntaxHighlighter syn4("float a = 0x10.1p0;");
  syn4.ParseCode();
  syn4.CheckSyntaxTokens({{TYPE_KEYWORD, 0, 5},
                          {WHITESPACE, 5, 6},
                          {IDENTIFIER, 6, 7},
                          {WHITESPACE, 7, 8},
                          {OPERATOR, 8, 9},
                          {WHITESPACE, 9, 10},
                          {NUMERIC_LITERAL, 10, 18},
                          {PUNCTUATION, 18, 19}});

  MockSyntaxHighlighter syn5("float a = 123.456e-67;");
  syn5.ParseCode();
  syn5.CheckSyntaxTokens({{TYPE_KEYWORD, 0, 5},
                          {WHITESPACE, 5, 6},
                          {IDENTIFIER, 6, 7},
                          {WHITESPACE, 7, 8},
                          {OPERATOR, 8, 9},
                          {WHITESPACE, 9, 10},
                          {NUMERIC_LITERAL, 10, 21},
                          {PUNCTUATION, 21, 22}});
}

TEST(SyntaxHighlightTest, CppStatements) {
  MockSyntaxHighlighter syn(R"(int a;if(a+1>3){a++;})");
  syn.ParseCode();
  syn.CheckSyntaxTokens({{TYPE_KEYWORD, 0, 3},
                         {WHITESPACE, 3, 4},
                         {IDENTIFIER, 4, 5},
                         {PUNCTUATION, 5, 6},
                         {KEYWORD, 6, 8},
                         {PARENTHESES, 8, 9},
                         {IDENTIFIER, 9, 10},
                         {OPERATOR, 10, 11},
                         {NUMERIC_LITERAL, 11, 12},
                         {OPERATOR, 12, 13},
                         {NUMERIC_LITERAL, 13, 14},
                         {PARENTHESES, 14, 15},
                         {BRACE, 15, 16},
                         {IDENTIFIER, 16, 17},
                         {OPERATOR, 17, 19},
                         {PUNCTUATION, 19, 20},
                         {BRACE, 20, 21}});
}

TEST(SyntaxHighlightTest, CppComments) {
  MockSyntaxHighlighter syn("abc; // Do something\ndef;");
  syn.ParseCode();
  syn.CheckSyntaxTokens({{IDENTIFIER, 0, 3},
                         {PUNCTUATION, 3, 4},
                         {WHITESPACE, 4, 5},
                         {COMMENT, 5, 20},
                         {WHITESPACE, 20, 21},
                         {IDENTIFIER, 21, 24},
                         {PUNCTUATION, 24, 25}});

  MockSyntaxHighlighter syn2("abc; /* this is some comment */ asdf;");
  syn2.ParseCode();
  syn2.CheckSyntaxTokens({{IDENTIFIER, 0, 3},
                          {PUNCTUATION, 3, 4},
                          {WHITESPACE, 4, 5},
                          {COMMENT, 5, 31},
                          {WHITESPACE, 31, 32},
                          {IDENTIFIER, 32, 36},
                          {PUNCTUATION, 36, 37}});
}

TEST(SyntaxHighlightTest, FunctionTest) {
  MockSyntaxHighlighter syn("int x = str.length()");
  syn.ParseCode();
  syn.CheckSyntaxTokens({{TYPE_KEYWORD, 0, 3},
                         {WHITESPACE, 3, 4},
                         {IDENTIFIER, 4, 5},
                         {WHITESPACE, 5, 6},
                         {OPERATOR, 6, 7},
                         {WHITESPACE, 7, 8},
                         {IDENTIFIER, 8, 11},
                         {OPERATOR, 11, 12},
                         {FUNCTION, 12, 18},
                         {PARENTHESES, 18, 20}});

  MockSyntaxHighlighter syn2("int x ( a ) {}");
  syn2.ParseCode();
  syn2.CheckSyntaxTokens({
      {TYPE_KEYWORD, 0, 3},
      {WHITESPACE, 3, 4},
      {FUNCTION, 4, 5},
      {WHITESPACE, 5, 6},
      {PARENTHESES, 6, 7},
      {WHITESPACE, 7, 8},
      {IDENTIFIER, 8, 9},
      {WHITESPACE, 9, 10},
      {PARENTHESES, 10, 11},
      {WHITESPACE, 11, 12},
      {BRACE, 12, 14},
  });
}

TEST(PySyntaxHighlightTest, BuiltInTest) {
  MockPySyntaxHighlighter syn("for i in range(x):");
  syn.ParseCode();
  syn.CheckSyntaxTokens({{KEYWORD, 0, 3},
                         {WHITESPACE, 3, 4},
                         {IDENTIFIER, 4, 5},
                         {WHITESPACE, 5, 6},
                         {KEYWORD, 6, 8},
                         {WHITESPACE, 8, 9},
                         {BUILT_IN, 9, 14},
                         {PARENTHESES, 14, 15},
                         {IDENTIFIER, 15, 16},
                         {PARENTHESES, 16, 17},
                         {OPERATOR, 17, 18}});
}

TEST(PySyntaxHighlightTest, PyComment) {
  MockPySyntaxHighlighter syn("print(a) # Comment\nprint(c)");
  syn.ParseCode();
  syn.CheckSyntaxTokens({{BUILT_IN, 0, 5},
                         {PARENTHESES, 5, 6},
                         {IDENTIFIER, 6, 7},
                         {PARENTHESES, 7, 8},
                         {WHITESPACE, 8, 9},
                         {COMMENT, 9, 18},
                         {WHITESPACE, 18, 19},
                         {BUILT_IN, 19, 24},
                         {PARENTHESES, 24, 25},
                         {IDENTIFIER, 25, 26},
                         {PARENTHESES, 26, 27}});
}

TEST(PySyntaxHighlightTest, PyLongString) {
  MockPySyntaxHighlighter syn("''' this is long string'''\nprint(a)");
  syn.ParseCode();
  syn.CheckSyntaxTokens({{STRING_LITERAL, 0, 26},
                         {WHITESPACE, 26, 27},
                         {BUILT_IN, 27, 32},
                         {PARENTHESES, 32, 33},
                         {IDENTIFIER, 33, 34},
                         {PARENTHESES, 34, 35}});
}

TEST(PySyntaxHighlightTest, PyMagicFunction) {
  MockPySyntaxHighlighter syn("def __init__(self):");
  syn.ParseCode();
  syn.CheckSyntaxTokens({{KEYWORD, 0, 3},
                         {WHITESPACE, 3, 4},
                         {MAGIC_FUNCTION, 4, 12},
                         {PARENTHESES, 12, 13},
                         {KEYWORD, 13, 17},
                         {PARENTHESES, 17, 18},
                         {OPERATOR, 18, 19}});
}

TEST(PySyntaxHighlightTest, PyBuiltIn2) {
  MockPySyntaxHighlighter syn("return ord('0')");
  syn.ParseCode();
  syn.CheckSyntaxTokens({{KEYWORD, 0, 6},
                         {WHITESPACE, 6, 7},
                         {BUILT_IN, 7, 10},
                         {PARENTHESES, 10, 11},
                         {STRING_LITERAL, 11, 14},
                         {PARENTHESES, 14, 15}});
}

class MockAsmSyntaxHighlighter
    : public AsmSyntaxHighlighter,
      public SyntaxHighlighterTester<MockAsmSyntaxHighlighter> {
 public:
  MockAsmSyntaxHighlighter(std::string_view content)
      : AsmSyntaxHighlighter(fake_context_, content, "asm") {}
};

TEST(AsmSyntaxHighlighter, LabelTest) {
  MockAsmSyntaxHighlighter syn(R"(label1:
label2:
label3:)");
  syn.ParseCode();
  syn.CheckSyntaxTokens({{LABEL, 0, 7},
                         {WHITESPACE, 7, 8},
                         {LABEL, 8, 15},
                         {WHITESPACE, 15, 16},
                         {LABEL, 16, 23}});
}

TEST(AsmSyntaxHighlighter, InstructionTest) {
  MockAsmSyntaxHighlighter syn(R"(func1:
mov eax, ebp)");
  syn.ParseCode();
  syn.CheckSyntaxTokens({
      {LABEL, 0, 6},
      {WHITESPACE, 6, 7},
      {INSTRUCTION, 7, 10},
      {WHITESPACE, 10, 11},
      {REGISTER, 11, 14},
      {PUNCTUATION, 14, 15},
      {WHITESPACE, 15, 16},
      {REGISTER, 16, 19},
  });
}

TEST(AsmSyntaxHighlighter, RegisterName) {
  MockAsmSyntaxHighlighter syn(R"(mov    0x0(%rip),%eax)");
  syn.ParseCode();
  syn.CheckSyntaxTokens({
      {INSTRUCTION, 0, 3},
      {WHITESPACE, 3, 7},
      {NUMERIC_LITERAL, 7, 10},
      {BRACKET, 10, 11},
      {REGISTER, 11, 15},
      {BRACKET, 15, 16},
      {PUNCTUATION, 16, 17},
      {REGISTER, 17, 21},
  });
}

TEST(AsmSyntaxHighlighter, RegisterAndIdentifier) {
  MockAsmSyntaxHighlighter syn(R"(mov BYTE PTR data[rbx-1], al)");
  syn.ParseCode();
  syn.CheckSyntaxTokens({
      {INSTRUCTION, 0, 3},
      {WHITESPACE, 3, 4},
      {IDENTIFIER, 4, 8},
      {WHITESPACE, 8, 9},
      {IDENTIFIER, 9, 12},
      {WHITESPACE, 12, 13},
      {IDENTIFIER, 13, 17},
      {BRACKET, 17, 18},
      {REGISTER, 18, 21},
      {OPERATOR, 21, 22},
      {NUMERIC_LITERAL, 22, 23},
      {BRACKET, 23, 24},
      {PUNCTUATION, 24, 25},
      {WHITESPACE, 25, 26},
      {REGISTER, 26, 28},
  });

  MockAsmSyntaxHighlighter syn2("imul rax, rax, 1374389535");
  syn2.ParseCode();
  syn2.CheckSyntaxTokens({{INSTRUCTION, 0, 4},
                          {WHITESPACE, 4, 5},
                          {REGISTER, 5, 8},
                          {PUNCTUATION, 8, 9},
                          {WHITESPACE, 9, 10},
                          {REGISTER, 10, 13},
                          {PUNCTUATION, 13, 14},
                          {WHITESPACE, 14, 15},
                          {NUMERIC_LITERAL, 15, 25}});
}

TEST(AsmSyntaxHighlighter, Directive) {
  MockAsmSyntaxHighlighter syn("jne .L2");
  syn.ParseCode();
  syn.CheckSyntaxTokens({
      {INSTRUCTION, 0, 3},
      {WHITESPACE, 3, 4},
      {DIRECTIVE, 4, 7},
  });
}

class MockObjdumpHighlighter
    : public ObjdumpHighlighter,
      public SyntaxHighlighterTester<MockObjdumpHighlighter> {
 public:
  MockObjdumpHighlighter(std::string_view content)
      : ObjdumpHighlighter(fake_context_, content, "objdump") {}
};

TEST(ObjdumpHighlighter, FunctionSection) {
  MockObjdumpHighlighter syn("0000000000001290 <__libc_csu_fini>:");
  syn.ParseCode();
  syn.CheckSyntaxTokens({
      {NUMERIC_LITERAL, 0, 16},
      {WHITESPACE, 16, 17},
      {FUNCTION_SECTION, 17, 35},
  });
}

TEST(ObjdumpHighlighter, InstructionSection) {
  MockObjdumpHighlighter syn(
      "    1294:       48 83 ec 08             sub    $0x8,%rsp");
  syn.ParseCode();
  syn.CheckSyntaxTokens({
      {WHITESPACE, 0, 4},
      {NUMERIC_LITERAL, 4, 9},
      {WHITESPACE, 9, 16},
      {NUMERIC_LITERAL, 16, 18},
      {WHITESPACE, 18, 19},
      {NUMERIC_LITERAL, 19, 21},
      {WHITESPACE, 21, 22},
      {NUMERIC_LITERAL, 22, 24},
      {WHITESPACE, 24, 25},
      {NUMERIC_LITERAL, 25, 27},
      {WHITESPACE, 27, 40},
      {INSTRUCTION, 40, 43},
      {WHITESPACE, 43, 47},
      {NUMERIC_LITERAL, 47, 51},
      {PUNCTUATION, 51, 52},
      {REGISTER, 52, 56},
  });
}

TEST(ObjdumpHighlighter, InstructionSection2) {
  MockObjdumpHighlighter syn(R"(
 objdump -S s.o 

s.o:     file format elf64-x86-64


Disassembly of section .text:

0000000000000000 <_Z4funcv>:
   0:	f3 0f 1e fa          	endbr64 
   4:	55                   	push   %rbp
   5:	48 89 e5             	mov    %rsp,%rbp
   8:	90                   	nop
   9:	5d                   	pop    %rbp
   a:	c3                   	retq   

000000000000000b <_ZL5func2v>:
   b:	f3 0f 1e fa          	endbr64 
   f:	55                   	push   %rbp
  10:	48 89 e5             	mov    %rsp,%rbp
  13:	90                   	nop
  14:	5d                   	pop    %rbp
  15:	c3                   	retq   
)");
  syn.ParseCode();
}

}  // namespace md2
