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

The first step was getting the UR3 running in simulation. The robot is described by a **URDF/xacro** model (its links, joints, limits and meshes), which also loads the `gazebo_ros2_control` plugin so that the standard ros2_control controllers can drive the simulated arm just like a real one.

A single launch file (`ur_sim_control.launch.py`) starts everything needed: Gazebo with the robot spawned in it, RViz for visualization, and the nodes that publish the robot's joint states and transforms, along with the joint trajectory controller. To move the arm, `gazebo_motion.py` publishes a `JointTrajectory` message, a list of joint-angle waypoints with a time for each, which the controller then follows. `joint_points_act_service.py` does the same thing through the `FollowJointTrajectory` action server.

### 2. MoveIt motion planning

Commanding joint angles directly works, but in practice you usually want the end-effector to reach a specific position and orientation. For this, the UR3 was fitted with a **2-jaw gripper** (the Panda hand) and set up in **MoveIt 2** using the MoveIt Setup Assistant (`ur3_panda_hand_moveit_config`). The configuration defines planning groups for the arm, the gripper and both together, uses the KDL kinematics solver, and connects MoveIt to the arm and gripper controllers.

Given a target **end-effector pose**, MoveIt solves the inverse kinematics, plans a collision-free trajectory and executes it. This is done from C++ in `hello_moveit.cpp`, which uses the `MoveGroupInterface` to set the target, plan and execute. A second node, `montiored_planning_scene_subscriber.cpp`, listens while the trajectory runs and saves the joint angles MoveIt computes at each waypoint to a **CSV file**, which is later used to move the real robot.

### 3. Real UR3 hardware

The physical UR3 is controlled from ROS 2 using the **Universal Robots ROS 2 driver**. The **External Control URCap** is installed on the robot's teach pendant and pointed at the host PC, which lets ROS send commands to the robot:

```bash
ros2 launch ur_robot_driver ur_control.launch.py ur_type:=ur3 robot_ip:=192.168.1.11 launch_rviz:=true
```

Once the external control program is running on the pendant, a goal publisher sends a sequence of joint-angle targets (listed in `test_goal_publishers_config.yaml`) to the trajectory controller. These targets come from the CSV recorded in the MoveIt step, so a motion can be **planned and checked in simulation first, then run on the real robot**. The `simple_moveit2_universal_robots_movement/` package also adds MoveIt nodes for moving the real arm to a pose and reading back its current end-effector pose.

🎥 **Demo:** [real UR3 video above](#demo-videos)

---

## Part B – Contour following and glue dispensing (RoboDK)

The goal here was to have the UR3 automatically trace the outline of an object and dispense glue along it, the way it would on a production line.

**Contour detection.** An Intel RealSense D435 depth camera is mounted above the gripper, looking down at the workspace. `counteract_reducing_no_of_points.py` captures a color and depth frame, converts the image to black and white with OpenCV, and picks out the largest contour, which is the object's outline. The raw contour has so many points that the arm's motion becomes jerky, so every few neighboring points are merged into one (using their median) to give a smoother path. The height comes from the depth image, and the final `(x, y, z, speed)` points are saved to a CSV file.

**Contour following.** The UR3 is modeled in **RoboDK**, with a work frame that acts as the origin for the contour points. `ImportCSV_XYZ_reducing_no_of_points.py` uses the RoboDK Python API to load the CSV, move the robot to a home position, trace the contour with straight-line moves at the speed given for each point, and return home. RoboDK handles the inverse kinematics along the way.

**Glue dispensing.** For the actual application, a glue tube is attached to the end-effector and the dispenser is wired to the robot's digital outputs. `ImportCSV_XYZ_1_DO_control_reducing_no_of_points.py` follows the same path, but switches the dispenser on just before tracing begins and off right after it ends, so glue is applied only along the contour.

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
