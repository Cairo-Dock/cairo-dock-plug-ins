
########### install files ###############

if ("${RUBY_FOUND}" STREQUAL "TRUE")
	install (FILES ${CMAKE_CURRENT_BINARY_DIR}/CDApplet.rb
        DESTINATION ${RUBY_LIB_DIR})
endif()

