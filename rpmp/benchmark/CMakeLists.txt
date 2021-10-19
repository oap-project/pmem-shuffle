add_executable(put_and_get put_and_get.cc)
target_link_libraries(put_and_get pmpool_client_jni)

add_executable(put put.cc)
target_link_libraries(put pmpool_client_jni)

add_executable(get get.cc)
target_link_libraries(get pmpool_client_jni)