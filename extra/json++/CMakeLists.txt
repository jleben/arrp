
add_compile_options(-Wno-unused-variable -Wno-unused-function)

set(src
  json_st.cc
  json.tab.cc
  lex.yy.cc
)

add_library(json STATIC ${src})
