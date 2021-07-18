#ifndef SYNTAX_C_H_
#define SYNTAX_C_H_

char *C_HL_extensions[] = { ".c", ".h", NULL };

char *C_HL_keywords[] = {
  "typedef", "struct", "switch",
  "case", "if", "else", "for", "do", "while", "break", "continue",
  "sizeof", "NULL", "return", "static", "enum", "default"

  "void|", "int|", "char|", "float|", "double|", "unsigned|", "signed|", 
  "long|", "#include|", "#ifndef|", "#define|", "#ifdef|", "#endif|", NULL
};

#define C_SYNTAX { \
  "c", \
  C_HL_extensions, \
  C_HL_keywords, \
  "//", \
  "/*", \
  "*/", \
  HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}

#endif
