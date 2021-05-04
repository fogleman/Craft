project(craft)

macro(add_sphinx_target TARGET_NAME BUILDER COMMENT_STR)
  add_custom_target(${TARGET_NAME}
    COMMAND sphinx-build -b ${BUILDER} . sphinx/build/${BUILDER}
    DEPENDS doxygen
    COMMENT ${COMMENT_STR})
endmacro(add_sphinx_target)

add_custom_target(doxygen
  COMMAND doxygen doxygen.conf
  COMMENT "Build doxygen xml files used by sphinx/breathe.")

add_sphinx_target(doc-html
  html
  "Build html documentation")

add_sphinx_target(doc-man
  man
  "Build MAN pages")

add_sphinx_target(doc-tex
  latex
  "Build PDF documentation")

add_custom_target(doc-all
  DEPENDS doc-html doc-man doc-tex)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/sphinx/build/man/craft.1
  DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man1)
