CONFIGURE_FILE(pearyd.service.in pearyd.service @ONLY)

# allow usage both as system or user service
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/pearyd.conf DESTINATION etc)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/pearyd.service DESTINATION lib/systemd/system)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/pearyd.service DESTINATION lib/systemd/user)
