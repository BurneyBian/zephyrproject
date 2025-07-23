# modules/adapter/CMakeLists.txt
zephyr_library()

set(OSWL_MODULE_SRC
	./mmz/sa_oswl_mmz.c
	./oswl_test.c
)

zephyr_library_sources(${OSWL_MODULE_SRC})
zephyr_include_directories(./)
