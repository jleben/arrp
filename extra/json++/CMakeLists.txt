set(src
  json_st.cc
  json.tab.cc
  lex.yy.cc
)

add_library(json STATIC ${src})
