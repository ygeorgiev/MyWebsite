add_library(mywebsite_module_admin MODULE admin.c)
target_link_libraries(mywebsite_module_admin mywebsite_utils)

install(TARGETS mywebsite_module_admin LIBRARY DESTINATION lib)
