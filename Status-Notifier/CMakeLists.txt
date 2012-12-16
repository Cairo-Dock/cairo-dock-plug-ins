
add_subdirectory(src)
add_subdirectory(data)

if (PYTHON_FOUND)
	add_subdirectory(watcher)
endif()
