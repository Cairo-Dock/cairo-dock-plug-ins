add_subdirectory(src)
add_subdirectory(data)

if ("${PYTHON_FOUND}" STREQUAL "TRUE")
	add_subdirectory(registrar)
endif()
