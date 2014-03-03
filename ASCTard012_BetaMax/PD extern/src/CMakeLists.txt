project(asctArdtoPD)
 
add_definitions(-DMSW)
 
set(SRC asctArdtoPD.c)
 
include_directories("C:/Program Files (x86)/pd/include")
link_directories("C:/Program Files (x86)/pd/bin")
 
add_library(asctArdtoPD SHARED ${SRC} asctArdtoPD.def)
 
target_link_libraries (asctArdtoPD pd)