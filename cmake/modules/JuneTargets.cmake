#
# newJuneTarget(
# 	name
#  	source1 [source2 source3 ...]
#   
#	 	[LINK_LIBS [lib1 lib2 lib3 ...]]
#	)
#
# name
# 	Name of target
#
# sources...
#		Sources for target
#
# LINK_LIBS libs...
#   Libraries to link with
#
function(newJuneTarget targetName)
  cmake_parse_arguments(NJT "STATIC" "INCLUDE_INSTALL_DIR;LIBRARY_INSTALL_DIR" "LINK_LIBS" ${ARGN})

  # Gets all the sources listed in the arguments
  set(NJT_SOURCES ${NJT_UNPARSED_ARGUMENTS})

  get_filename_component(base_dir ${CMAKE_CURRENT_SOURCE_DIR} NAME)

  # Recursively gets all the headers
  file(GLOB_RECURSE NJT_HEADERS
      ${PROJECT_SOURCE_DIR}/include/${base_dir}/*.h
      ${PROJECT_SOURCE_DIR}/include/*.h)
  
  # Combine all sources and headers into one list
  set(NJT_ALL_SOURCES ${NJT_SOURCES} ${NJT_HEADERS})

  if (NJT_STATIC)
    # Static library
    add_library(
      ${targetName}
      ${NJT_ALL_SOURCES}
      )
  else
    # Shared library
    add_library(
      ${targetName}
      ${NJT_ALL_SOURCES}
      SHARED
      )
  endif()

  # Add the libraries
  if (NJT_LINK_LIBS)
    target_link_libraries(
      ${targetName}
      ${NJT_LINK_LIBS}
      ${CMAKE_DL_LIBS}
      ${MPFR_LIBRARIES}
      ${GMP_LIBRARY}
    )
  endif()

  set_target_properties(
    ${targetName}
    PROPERTIES
    OUTPUT_NAME ${targetName}
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${NJT_LIBRARY_INSTALL_DIR}"
    INSTALL_RPATH_USE_LINK_PATH ON
  )

  install(
    TARGETS ${targetName}
    RUNTIME
      DESTINATION lib/${NJT_LIBRARY_INSTALL_DIR}
      COMPONENT Libraries
    PUBLIC_HEADER
	    DESTINATION include/${NJT_INCLUDE_INSTALL_DIR}
  )

  if (EXISTS "${CMAKE_BINARY_DIR}/compile_commands.json")
    add_custom_command(
      TARGET ${name}
      POST_BUILD
      COMMAND cp ${CMAKE_BINARY_DIR}/compile_commands.json ${PROJECT_SOURCE_DIR}
      VERBATIM
    )
  endif()
endfunction()

