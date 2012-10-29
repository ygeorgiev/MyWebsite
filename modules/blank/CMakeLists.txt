add_library(mywebsite_module_blank MODULE blank.c)
target_link_libraries(mywebsite_module_blank mywebsite_utils)

install(TARGETS mywebsite_module_blank LIBRARY DESTINATION lib)
