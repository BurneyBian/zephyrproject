# modules/adapter/CMakeLists.txt
zephyr_library()

set(OSAL_MODULE_SRC
	./osal.c
	./osal_addr.c
	./osal_atomic.c
	./osal_barrier.c
	./osal_cache.c
	./osal_fileops.c
	./osal_heap.c
	./osal_interrupt.c
	./osal_math.c
	./osal_mutex.c
	./osal_notifier.c
	./osal_vmalloc.c
	./osal_proc.c
	./osal_schedule.c
	./osal_semaphore.c
	./osal_spinlock.c
	./osal_string.c
	./osal_task.c
	./osal_timer.c
	./osal_unsupport.c
	./osal_wait.c
	./osal_workqueue.c
	./osal_print.c
	./test/osal_test.c
	./devfs/devfs.c
	./osal_device.c
	./mmz_drv/mmz-userdev.c
	./mmz_drv/mmz_bestfit.c
	./acme/acme_osal_interface.c
)

zephyr_library_sources(${OSAL_MODULE_SRC})
zephyr_library_sources_ifdef(CONFIG_FILE_SYSTEM ./procfs/procfs.c)
zephyr_include_directories(./)
