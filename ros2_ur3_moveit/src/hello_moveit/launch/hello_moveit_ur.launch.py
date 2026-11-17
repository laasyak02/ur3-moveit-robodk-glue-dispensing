import launch
import os
import sys

from launch_ros.actions import Node
from launch.substitutions import PathJoinSubstitution, Command, FindExecutable
from launch_ros.substitutions import FindPackageShare
from moveit_configs_utils import MoveItConfigsBuilder

'''
def get_robot_description():
    robot_description_content = Command(
        [
            PathJoinSubstitution([FindPackageShare("moveit_resources_panda_description"), "urdf", "ur3_panda_gripper.urdf"])
        ]
    )

    robot_description = {"robot_description": robot_description_content}
    return robot_description

def get_robot_description_semantic():
    # MoveIt Configuration
    robot_description_semantic_content = Command(
        [
            PathJoinSubstitution([FindPackageShare("ur3_panda_hand_moveit_config"), "config", "ur3_robot.srdf"]),
            " ",
            # Also ur_type parameter could be used but then the planning group names in yaml
            # configs has to be updated!
            "prefix:=",
            '""',
            " ",
        ]
    )
    robot_description_semantic = {
        "robot_description_semantic": robot_description_semantic_content
    }
    return robot_description_semantic
    
'''
def generate_launch_description():

    moveit_config = (
        MoveItConfigsBuilder("ur3_panda_hand")
        .robot_description(
            file_path="config/ur3_robot.urdf.xacro"
        )
        .robot_description_semantic(file_path="config/ur3_robot.srdf")
        .robot_description_kinematics(file_path="config/kinematics.yaml")
        .to_moveit_configs()
    )
            
    # generate_common_hybrid_launch_description() returns a list of nodes to launch
    #robot_description = get_robot_description()
    #robot_description_semantic = get_robot_description_semantic()
    demo_node = Node(
        package="hello_moveit",
        executable="hello_moveit",
        name="hello_moveit",
        output="screen",
        parameters=[
            moveit_config.robot_description,
            moveit_config.robot_description_semantic,
            moveit_config.robot_description_kinematics,
            {"use_sim_time": True},
        ],
    )
    return launch.LaunchDescription([demo_node])
