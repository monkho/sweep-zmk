zephyr_library()
zephyr_library_include_directories(boards/shields/dongle_screen/include)

if(CONFIG_SHIELD_DONGLE_SCREEN)

        add_subdirectory(${ZEPHYR_CURRENT_MODULE_DIR}/drivers/display)

endif()