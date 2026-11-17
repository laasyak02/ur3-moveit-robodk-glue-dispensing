cmake_minimum_required(VERSION 3.5)
project(ur_simulation_gazebo)

find_package(ament_cmake REQUIRED)

install(DIRECTORY config launch
  DESTINATION share/${PROJECT_NAME}
)

install(PROGRAMS scripts/joint_points_act_service.py 
  DESTINATION lib/${PROJECT_NAME}
)

install(PROGRAMS scripts/gazebo_motion.py
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
