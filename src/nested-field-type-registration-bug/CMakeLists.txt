add_executable(pub "src/HelloWorldPublisher.cpp")
target_link_libraries(pub fastcdr fastrtps)

add_executable(sub "src/HelloWorldSubscriber.cpp")
target_link_libraries(sub fastcdr fastrtps)

add_executable(contrived_sub "src/ContrivedSubscriber.cpp")
target_link_libraries(contrived_sub fastcdr fastrtps)
