#!/usr/bin/env python3
import rclpy
from trajectory_msgs.msg import JointTrajectory, JointTrajectoryPoint

def main():
    rclpy.init()
    node = rclpy.create_node('joint_trajectory_publisher')

    # Create a publisher for the joint trajectory
    joint_trajectory_publisher = node.create_publisher(JointTrajectory, '/joint_trajectory_controller/joint_trajectory', 10)

    # Create a JointTrajectory message
    joint_trajectory_msg = JointTrajectory()
    joint_trajectory_msg.joint_names = ['shoulder_pan_joint', 'shoulder_lift_joint', 'elbow_joint', 'wrist_1_joint', 'wrist_2_joint', 'wrist_3_joint']

    # Create a trajectory point with desired joint positions
    trajectory_point = JointTrajectoryPoint()
    trajectory_point.positions = [0.0, -1.57, -0.5, -1.57, 0.0, 0.0] 
    trajectory_point.time_from_start = rclpy.duration.Duration(seconds=1, nanoseconds=0).to_msg()  # Set the time duration for the trajectory
    
    trajectory_point_2 = JointTrajectoryPoint()
    trajectory_point_2.positions = [0.0,-1.0,0.0,-1.6,0.0, 0.0] 
    trajectory_point_2.time_from_start = rclpy.duration.Duration(seconds=1, nanoseconds=0).to_msg()  # Set the time duration for the trajectory
    
    trajectory_point_3 = JointTrajectoryPoint()
    trajectory_point_3.positions = [6.28,-1.0,0.0,-1.6,0.0, 0.0] 
    trajectory_point_3.time_from_start = rclpy.duration.Duration(seconds=3, nanoseconds=0).to_msg()  # Set the time duration for the trajectory


    # Add the trajectory point to the joint trajectory
    joint_trajectory_msg.points.append(trajectory_point_2)
    joint_trajectory_msg.points.append(trajectory_point_3)

    # Publish the joint trajectory
    joint_trajectory_publisher.publish(joint_trajectory_msg)

    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
