
SET(impl_SRCS
	cairo-dock-gio-vfs.c				cairo-dock-gio-vfs.h
)

if (${LIBGIO_FOUND})
	add_definitions (-DHAVE_LIBGIO="1")
	add_definitions (-fPIC)
endif()

add_library(gvfs-integration STATIC ${impl_SRCS})

link_directories(
	${PACKAGE_LIBRARY_DIRS}
	${LIBGIO_LIBRARY_DIRS})

include_directories(
	${PACKAGE_INCLUDE_DIRS}
	${LIBGIO_INCLUDE_DIRS})

target_link_libraries (gvfs-integration
	${PACKAGE_LIBRARIES}
	${LIBGIO_LIBRARIES})

########### install files ###############
