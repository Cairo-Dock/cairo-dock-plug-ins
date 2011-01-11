
########### install files ###############

if ("${PYTHON_FOUND}" STREQUAL "TRUE")
	install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/CDApplet.py
        DESTINATION ${PYTHON_SITE_PACKAGE})
endif()

