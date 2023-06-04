set(LIB_SOURCES
  tmalloc.c
  logger.c
)
include_directories("../include")

set(HEADERS 
    ../include/tmalloc.h
    logger.h
)

add_library(tmem STATIC ${LIB_SOURCES} ${HEADERS})

install(TARGETS tmem
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

install(FILES ${HEADERS} DESTINATION include)