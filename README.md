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

Getting the UR3 running in simulation, driven by the same ros2_control controllers used on real robots.

- **Robot model:** the UR3 is described by a URDF/xacro model (links, joints, limits and meshes). It loads the `gazebo_ros2_control` plugin, which lets ros2_control drive the simulated arm.
- **Launch:** `ur_sim_control.launch.py` starts Gazebo with the UR3 spawned in it, along with RViz, the joint state and robot state publishers, and the joint trajectory controller.
- **Moving the arm:** `gazebo_motion.py` publishes a `JointTrajectory`, which is a list of joint-angle waypoints with a time for each, and the controller follows it. `joint_points_act_service.py` does the same through the `FollowJointTrajectory` action.

### 2. MoveIt motion planning

Moving the end-effector to a target position and orientation instead of commanding each joint by hand.

- **Robot setup:** the UR3 is fitted with a 2-jaw gripper (the Panda hand) and configured in **MoveIt 2** with the MoveIt Setup Assistant (`ur3_panda_hand_moveit_config`).
- **Configuration:** there are planning groups for the arm, the gripper and both together. The config uses the KDL inverse-kinematics solver and connects MoveIt to the arm and gripper controllers.
- **Planning to a pose:** `hello_moveit.cpp` uses the `MoveGroupInterface` to set a target end-effector pose. MoveIt solves the inverse kinematics, plans a collision-free trajectory, and executes it.
- **Recording the motion:** `montiored_planning_scene_subscriber.cpp` saves the joint angles at each waypoint of the planned trajectory to a CSV file. These are later used to drive the real robot.

### 3. Real UR3 hardware

Running motions on the physical UR3 from ROS 2.

- **Connection:** the robot is controlled with the Universal Robots ROS 2 driver. The **External Control URCap** on the teach pendant lets ROS send it commands:
  ```bash
  ros2 launch ur_robot_driver ur_control.launch.py ur_type:=ur3 robot_ip:=192.168.1.11 launch_rviz:=true
  ```
- **Sending goals:** a goal publisher sends a sequence of joint-angle targets from `test_goal_publishers_config.yaml` to the robot's trajectory controller.
- **Simulation to hardware:** these targets come from the CSV recorded in the MoveIt step. A motion can be planned and checked in simulation first, then run on the real robot.
- **Extras:** `simple_moveit2_universal_robots_movement/` adds MoveIt nodes that move the real arm to a pose and read back its current end-effector pose.

🎥 **Demo:** [real UR3 video above](#demo-videos)

---

## Part B – Contour following and glue dispensing (RoboDK)

Having the UR3 automatically trace an object's outline and dispense glue along it, as it would on a production line.

- **Contour detection:** a RealSense D435 depth camera mounted above the gripper captures color and depth images of the workspace. `counteract_reducing_no_of_points.py` converts the image to black and white with OpenCV and picks out the largest contour, which is the object's outline.
- **Smoothing the path:** the raw contour has too many points, which makes the arm's motion jerky. Every few neighboring points are merged into one using their median. The height is taken from the depth image, and the final `(x, y, z, speed)` points are saved to a CSV file.
- **Contour following:** the UR3 is modeled in **RoboDK**, with a work frame that acts as the origin for the points. `ImportCSV_XYZ_reducing_no_of_points.py` loads the CSV, moves the robot to a home position, traces the contour with straight-line moves at each point's speed, and then returns home.
- **Glue dispensing:** a glue tube is attached to the end-effector, and the dispenser is wired to the robot's digital outputs. `ImportCSV_XYZ_1_DO_control_reducing_no_of_points.py` follows the same path, but turns the dispenser on just before tracing starts and off right after it ends, so glue is applied only along the contour.

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
