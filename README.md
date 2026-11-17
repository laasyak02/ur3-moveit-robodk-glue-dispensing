# UR3 Manipulator: ROS 2 Simulation, MoveIt Motion Planning, Hardware Control & RoboDK Glue Dispensing

Project work carried out at the **Central Manufacturing Technology Institute (CMTI), Bengaluru** - Smart Manufacturing Demo and Development Cell (SMDDC), Jan – Jun 2024.

This project focuses on simulating and controlling the **Universal Robots UR3**, a 6-DOF collaborative robot (cobot) with a 3 kg payload. It has two parts:

- **Part A – Simulating and controlling the UR3 with ROS 2:** simulation in Gazebo and RViz, motion planning with MoveIt, and control of the real UR3 hardware through the UR ROS 2 driver.
- **Part B – Contour following and glue dispensing with RoboDK:** an Intel RealSense D435 depth camera detects the outline of an object, and the UR3 traces that contour while dispensing glue along it, programmed through the RoboDK Python API.

**Tech stack:** ROS 2 Humble · Ubuntu 22.04 · MoveIt 2 · Gazebo Classic · RViz2 · ros2_control · C++ · Python · OpenCV · Intel RealSense (pyrealsense2) · RoboDK API

## 🎥 Demo videos

### 1. UR3 simulated in Gazebo and moved with joint trajectories

<!-- VIDEO: drag 1_gazebo_simulation.mp4 here -->

### 2. MoveIt motion planning in RViz + recording joint angles

<!-- VIDEO: drag 2_moveit_motion_planning.mp4 here -->

### 3. Real UR3 hardware controlled from ROS 2

<!-- VIDEO: drag 3_real_ur3_hardware.mp4 here -->

### 4. Contour detection + RoboDK contour following for glue dispensing

<!-- VIDEO: drag 4_robodk_glue_dispensing_1.mp4 here -->

<!-- VIDEO: drag 5_robodk_glue_dispensing_2.mp4 here -->

<sub>Full-quality originals: [Gazebo](https://drive.google.com/file/d/1PE3DwFaBpafk_dYYePK-26resAGDYOTn/view?usp=sharing) · [MoveIt](https://drive.google.com/file/d/11h6GLUL-r-kAuBZAxoEpLJ22V9rI8s3I/view?usp=sharing) · [Real UR3](https://drive.google.com/file/d/1zVEsSdLhUvHBWT_ES3-_VT3H8fEbtrh_/view?usp=sharing) · [RoboDK (1)](https://drive.google.com/file/d/1M3b55jrSM6bCR4SARstUZwAzJ8IIUkt2/view?usp=sharing) · [RoboDK (2)](https://drive.google.com/file/d/1ndm5ypgysjGRwmhZ_eQcjg_SJYLkwsUF/view?usp=sharing)</sub>

---

## Repository structure

| Folder | What it contains |
|---|---|
| [`ros2_ur3_gazebo_sim/`](ros2_ur3_gazebo_sim) | ROS 2 workspace – UR3 simulation in Gazebo and RViz, moved with joint-trajectory commands |
| [`ros2_ur3_moveit/`](ros2_ur3_moveit) | ROS 2 workspace – UR3 with a 2-finger gripper, MoveIt configuration, and C++ motion-planning nodes |
| [`ros2_ur3_hardware_control/`](ros2_ur3_hardware_control) | ROS 2 workspace – controlling the real UR3 with the Universal Robots ROS 2 driver |
| [`robodk_glue_dispensing/`](robodk_glue_dispensing) | Python scripts – RealSense contour detection and RoboDK contour following with glue-dispenser control |

Each `ros2_*` folder is an independent colcon workspace (`<workspace>/src/...`) and is built on its own.

---

## Part A – Simulating and Controlling the UR3 using ROS 2

### 1. UR3 simulation in Gazebo and RViz (`ros2_ur3_gazebo_sim`)

The UR3 is described with a **URDF/xacro** model that defines its six links (shoulder, upper arm, forearm, wrist 1–3) and six revolute joints, along with their limits, inertias, and visual and collision meshes. The URDF also loads the `gazebo_ros2_control` plugin, which connects the **ros2_control** controller framework to the Gazebo simulator.

A launch file starts the whole simulation together:
- **Robot State Publisher** – publishes the robot's link transforms (tf2) from the URDF and joint states
- **Joint State Broadcaster** – publishes `sensor_msgs/JointState` for all non-fixed joints
- **Joint trajectory controller** spawner – starts the controller that drives the arm
- **Gazebo** + **spawn_entity** – opens the simulator and spawns the UR3 into it
- **RViz2** – for visualization

The controllers (joint trajectory, scaled joint trajectory, forward velocity and force/torque sensor broadcaster) are configured in `ur_controllers.yaml`.

**Moving the robot:** `scripts/gazebo_motion.py` creates a `joint_trajectory_publisher` node that publishes a `trajectory_msgs/JointTrajectory` message to `/joint_trajectory_controller/joint_trajectory`. The message contains a sequence of joint-position waypoints (in radians) with times for each. `joint_points_act_service.py` sends the same kind of trajectory as a goal to the `FollowJointTrajectory` action server.

Key files (in `src/Universal_Robots_Gazebo_Simulation/Universal_Robots_ROS2_Gazebo_Simulation/ur_simulation_gazebo/`):
- `launch/ur_sim_control.launch.py`, `launch/ur_sim_moveit.launch.py`
- `config/ur_controllers.yaml`
- `scripts/gazebo_motion.py`, `scripts/joint_points_act_service.py`

🎥 **Demo:** [video 1 above](#-demo-videos)

### 2. Motion planning with MoveIt (`ros2_ur3_moveit`)

Here the UR3 is customized with a **2-jaw gripper end-effector** (the Panda hand) for pick-and-place, and the combined model is simulated in RViz with **MoveIt 2** motion planning. Instead of commanding joint angles directly, you give a desired **end-effector pose** (position + orientation). MoveIt solves the inverse kinematics with the **KDL kinematics solver**, plans a collision-free trajectory and executes it.

**MoveIt configuration** (generated with the MoveIt Setup Assistant, in `ur3_panda_hand_moveit_config/`):
- **SRDF** with three planning groups: `ur3_arm_pg` (the arm, `base_link` → `tool0`), `panda_hand` (the gripper) and `ur3_arm_hand` (both together). It also has named states such as `default`, `standing_up`, `hand_open` and `hand_closed`.
- `kinematics.yaml` – KDL solver settings
- `ros2_controllers.yaml` – `ur3_arm_pg_controller` (JointTrajectoryController), `panda_hand_controller` (GripperActionController) and the joint state broadcaster
- `moveit_controllers.yaml` – maps MoveIt to those controllers (`FollowJointTrajectory` and `GripperCommand`)
- `joint_limits.yaml`, `initial_positions.yaml`
- `launch/demo.launch.py` – starts RViz with the MoveIt planning pipeline

**C++ nodes** (in `hello_moveit/src/`):
- `hello_moveit.cpp` – uses the **MoveGroupInterface** for `ur3_arm_pg` to set a target pose, plan a trajectory to it and execute it if planning succeeds
- `montiored_planning_scene_subscriber.cpp` – subscribes to `/monitored_planning_scene` and `/execute_trajectory/_action/feedback`. While a trajectory is executing, it records the joint angles that MoveIt computes for each waypoint to a **CSV file**. Those joint values are then used to drive the real robot (see section 3).
- `joint_states_subscriber.cpp`, `pose_end_effector.cpp` – helper nodes that read joint states and the current end-effector pose

Other packages in this workspace: `ur3_moveit_config1` and `ur3_gripper_moveit_config` (alternative MoveIt configs for the bare UR3 and for a UR3 with a Robotiq gripper), `panda_description` (gripper meshes plus the combined `ur3_panda_gripper.urdf`), and `robotiq_arg85_description` (with an added `ur3_robotiq_gripper.urdf`).

🎥 **Demo:** [video 2 above](#-demo-videos)

### 3. Controlling the real UR3 hardware (`ros2_ur3_hardware_control`)

The physical UR3 is controlled from ROS 2 with the **Universal Robots ROS 2 driver** (`ur_robot_driver`, Humble). The **External Control URCap** is installed on the robot's teach pendant, with the host PC's IP and port configured, so that ROS can command the robot.

```bash
ros2 launch ur_robot_driver ur_control.launch.py ur_type:=ur3 robot_ip:=192.168.1.11 launch_rviz:=true
# optionally choose the controller:
#   initial_joint_controller:=joint_trajectory_controller   (default: scaled_joint_trajectory_controller)
```

After you start the `external_control` program on the pendant, a goal-publisher launch file sends the joint-angle goals listed in `ur_robot_driver/config/test_goal_publishers_config.yaml` to the chosen trajectory controller. Each goal (`pos1`…`pos4`) is a set of six joint angles in radians, and `check_starting_point` makes sure they stay within each joint's limits.

**Going from simulation to the real robot:** to reach a Cartesian target on the real robot, the motion is first planned in MoveIt (section 2), and the resulting joint angles are saved to CSV. Those values are then used as the position goals in this config file.

`simple_moveit2_universal_robots_movement/` contains MoveIt `MoveGroupInterface` nodes for the real robot's `ur_manipulator` group. `hello_moveit_ur.cpp` moves the robot to a target pose, and `pose_end_effector.cpp` reads and prints the current end-effector pose.

🎥 **Demo:** [video 3 above](#-demo-videos)

---

## Part B – Contour Following and Glue Dispensing using RealSense and RoboDK (`robodk_glue_dispensing`)

### Contour detection with the Intel RealSense D435
The D435 stereo depth camera is mounted above the UR3's gripper, facing down at the workspace. `counteract_reducing_no_of_points.py`:
1. Captures aligned **color and depth frames** with `pyrealsense2` and converts them to OpenCV images
2. Converts to **grayscale → binary**, then finds contours with `cv2.findContours` and keeps the **largest contour**
3. **Reduces and smooths the points.** The raw contour has so many points that the arm's motion becomes jerky, so each group of up to three neighboring points is replaced by their **median**.
4. Takes the Z coordinate from the depth image (the depth of the highest point is used for the whole contour) and writes the `(x, y, z, speed)` points to a CSV file

### Contour following in RoboDK
The UR3 is modeled in **RoboDK** with three reference frames: the robot base, the tool (TCP) and a work frame (Frame 2) that serves as the origin for the contour points. Using the **RoboDK Python API**, the scripts:
- Load the contour CSV and move the robot to a fixed home joint configuration (`MoveJ`)
- Trace the contour with linear moves (`MoveL`) through each point, applying the speed from each CSV row. RoboDK handles the inverse kinematics and converts Cartesian velocities to joint velocities through the inverse Jacobian.
- Return to the home position, ready for the next part

| Script | Purpose |
|---|---|
| `counteract_reducing_no_of_points.py` | RealSense capture + contour detection + point reduction → CSV |
| `ImportCSV_XYZ_reducing_no_of_points.py` | Loads the CSV and builds or runs the contour-following program in RoboDK, so the path can be saved as a reusable RoboDK program |
| `ImportCSV_XYZ_1_DO_control_reducing_no_of_points.py` | Same, plus **digital-output control of the glue dispenser** (`setDO`): dispensing turns on just before tracing starts and off right after it ends |

For the glue-dispensing application, a glue-holder tube is attached to the end-effector, and the dispenser module is wired to the robot's digital outputs. The three scripts run together as a single RoboDK program.

🎥 **Demos:** [videos 4 above](#-demo-videos)

---

## Building the ROS 2 workspaces

Requires **ROS 2 Humble** on Ubuntu 22.04, plus MoveIt 2, Gazebo Classic (`gazebo_ros_pkgs`, `gazebo_ros2_control`) and ros2_control.

```bash
cd ros2_ur3_moveit            # or ros2_ur3_gazebo_sim / ros2_ur3_hardware_control
rosdep install --from-paths src --ignore-src -r -y
colcon build --symlink-install
source install/setup.bash
```

Examples:
```bash
# Gazebo simulation, then move the arm
ros2 launch ur_simulation_gazebo ur_sim_control.launch.py ur_type:=ur3
python3 src/Universal_Robots_Gazebo_Simulation/Universal_Robots_ROS2_Gazebo_Simulation/ur_simulation_gazebo/scripts/gazebo_motion.py

# MoveIt demo with the UR3 + gripper in RViz, then plan to a pose
ros2 launch ur3_panda_hand_moveit_config demo.launch.py
ros2 run hello_moveit hello_moveit
```

The RoboDK scripts need RoboDK with its Python API (`robodk`), plus `opencv-python`, `numpy`, `scipy` and `pyrealsense2`. The CSV file paths inside the scripts are set to the original lab machine and need to be updated before you run them.

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

The `hello_moveit` node follows the [MoveIt 2 "Your First C++ MoveIt Project"](https://moveit.picknik.ai/humble/doc/tutorials/your_first_project/your_first_project.html) tutorial.

## Future work
- Optimize MoveIt planning for smoother, better-tuned trajectories
- Produce smoother, more continuous contours
- Full **3D contour following** that uses the depth of every contour point, so the path follows height changes on the object for more accurate glue dispensing

## Acknowledgements
Mentored by Harikrishna S Thota (Scientist D), C-SMPM, CMTI Bengaluru.
