# UR3 Manipulator: ROS 2 Simulation, MoveIt Motion Planning, Hardware Control & RoboDK Glue Dispensing

This project was carried out as part of an internship at the Central Manufacturing Technology Institute (CMTI), Bengaluru from Jan - Jun 2024. It focuses on simulating and controlling the **Universal Robots UR3**, a 6-DOF collaborative robot (cobot) with a 3 kg payload. It has two parts:

- **Part A – Simulating and controlling the UR3 with ROS 2:** simulation in Gazebo and RViz, motion planning with MoveIt, and control of the real UR3 hardware through the UR ROS 2 driver.
- **Part B – Contour following and glue dispensing with RoboDK:** an Intel RealSense D435 depth camera detects the outline of an object, and the UR3 traces that contour while dispensing glue along it, programmed through the RoboDK Python API.

## Demo videos

### Real UR3 hardware controlled from ROS 2

https://github.com/user-attachments/assets/8a413dd6-08f7-47d2-9824-7e47cafdabab

### Contour detection + RoboDK contour following for glue dispensing

<table>
  <tr>
    <td><video src="https://github.com/user-attachments/assets/c142b887-3fd6-4e01-82c5-cde47c66b90d" controls></video></td>
    <td><video src="https://github.com/user-attachments/assets/da7d888d-d346-4a67-a078-a5cf114c005b" controls></video></td>
  </tr>
</table>

---

## What's here

| Directory | Description |
|---|---|
| [`ros2_ur3_gazebo_sim/`](ros2_ur3_gazebo_sim) | UR3 simulation in Gazebo + RViz, driven by joint-trajectory commands |
| [`ros2_ur3_moveit/`](ros2_ur3_moveit) | UR3 + 2-finger gripper MoveIt config and C++ motion-planning nodes |
| [`ros2_ur3_hardware_control/`](ros2_ur3_hardware_control) | Control of the real UR3 through the Universal Robots ROS 2 driver |
| [`robodk_glue_dispensing/`](robodk_glue_dispensing) | RealSense contour detection and RoboDK contour following with glue-dispenser control |

Each `ros2_*` folder is an independent colcon workspace.

---

## Part A – UR3 with ROS 2

### 1. Gazebo simulation

- UR3 **URDF/xacro** model with the `gazebo_ros2_control` plugin, so ros2_control drives the simulated arm.
- One launch file (`ur_sim_control.launch.py`) brings up Gazebo, RViz, the robot state publisher, joint state broadcaster and joint trajectory controller.
- `gazebo_motion.py` publishes a `JointTrajectory` of joint-angle waypoints; `joint_points_act_service.py` sends the same through the `FollowJointTrajectory` action.

### 2. MoveIt motion planning

- UR3 fitted with a **2-jaw gripper** (Panda hand) and configured with the MoveIt Setup Assistant (`ur3_panda_hand_moveit_config`): arm, hand and combined planning groups, KDL kinematics, trajectory + gripper controllers.
- Targets are given as **end-effector poses**; MoveIt solves IK, plans a collision-free path and executes it.
- C++ nodes (`hello_moveit/`): `hello_moveit.cpp` plans and executes to a target pose via `MoveGroupInterface`; `montiored_planning_scene_subscriber.cpp` logs the planned joint angles to **CSV** for reuse on the real robot.

### 3. Real UR3 hardware

- The real UR3 runs the **External Control URCap** and is driven by `ur_robot_driver`:
  ```bash
  ros2 launch ur_robot_driver ur_control.launch.py ur_type:=ur3 robot_ip:=192.168.1.11 launch_rviz:=true
  ```
- Joint goals in `test_goal_publishers_config.yaml` are sent to the trajectory controller. These come from the MoveIt-planned CSV, so **planning happens in simulation and runs on hardware**.
- `simple_moveit2_universal_robots_movement/` adds MoveIt nodes for moving the real arm to a pose and reading back the end-effector pose.

🎥 **Demo:** [real UR3 video above](#demo-videos)

---

## Part B – Contour following and glue dispensing (RoboDK)

- **Contour detection** (`counteract_reducing_no_of_points.py`): a RealSense D435 above the gripper captures color + depth; OpenCV thresholds the image and keeps the largest contour. Neighboring points are median-merged to reduce jerky motion, and `(x, y, z, speed)` points are written to CSV.
- **Contour following** (`ImportCSV_XYZ_reducing_no_of_points.py`): the RoboDK Python API loads the CSV, moves home (`MoveJ`), traces the contour with `MoveL` at each point's speed, and returns home.
- **Glue dispensing** (`ImportCSV_XYZ_1_DO_control_reducing_no_of_points.py`): same path, with the dispenser switched on/off through the robot's **digital outputs** (`setDO`) at the start and end of the trace.

🎥 **Demo:** [glue-dispensing videos above](#demo-videos)

---

## Stack

`ROS 2 Humble` · `MoveIt 2` · `Gazebo Classic` · `RViz2` · `ros2_control` · `UR ROS 2 driver` · `RoboDK API` · `OpenCV` · `Intel RealSense` · `C++` · `Python`

---

## Build & run

**ROS 2 workspaces** (Ubuntu 22.04, ROS 2 Humble, MoveIt 2, Gazebo Classic):
```bash
cd ros2_ur3_moveit            # or ros2_ur3_gazebo_sim / ros2_ur3_hardware_control
rosdep install --from-paths src --ignore-src -r -y
colcon build --symlink-install
source install/setup.bash

ros2 launch ur3_panda_hand_moveit_config demo.launch.py   # MoveIt + RViz
ros2 run hello_moveit hello_moveit                        # plan to a pose
```

**RoboDK scripts:** need RoboDK with its Python API, plus `opencv-python`, `numpy`, `scipy` and `pyrealsense2`. Update the CSV paths in the scripts before running.

---

## Third-party packages

Several packages are based on open-source repositories, included here with the modifications made for this project:

| Package | Upstream |
|---|---|
| `Universal_Robots_ROS2_Description` | [UniversalRobots/Universal_Robots_ROS2_Description](https://github.com/UniversalRobots/Universal_Robots_ROS2_Description) |
| `Universal_Robots_ROS2_Gazebo_Simulation` | [UniversalRobots/Universal_Robots_ROS2_Gazebo_Simulation](https://github.com/UniversalRobots/Universal_Robots_ROS2_Gazebo_Simulation) |
| `Universal_Robots_ROS2_Driver` | [UniversalRobots/Universal_Robots_ROS2_Driver](https://github.com/UniversalRobots/Universal_Robots_ROS2_Driver) |
| `simple_moveit2_universal_robots_movement` | [LucaBross/simple_moveit2_universal_robots_movement](https://github.com/LucaBross/simple_moveit2_universal_robots_movement) |
| `robotiq_arg85_description` | [a-price/robotiq_arg85_description](https://github.com/a-price/robotiq_arg85_description) |
| `panda_description`, `panda_moveit_config` | [moveit_resources](https://github.com/ros-planning/moveit_resources) |
