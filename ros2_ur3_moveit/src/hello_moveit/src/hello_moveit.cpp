#include <memory>
#include <Eigen/Geometry>
#include <rclcpp/rclcpp.hpp>
#include <cstdlib>
#include <moveit/move_group_interface/move_group_interface.h>
#include <geometry_msgs/msg/pose.hpp>

int i;
Eigen::Quaterniond quaternion;
const std::string end_effector_link = "tool0";
int main(int argc, char * argv[])
{
  // Initialize ROS and create the Node
  rclcpp::init(argc, argv);
  auto const node = std::make_shared<rclcpp::Node>(
    "hello_moveit",    rclcpp::NodeOptions().automatically_declare_parameters_from_overrides(true)
  );
  //rclcpp::executors::SingleThreadedExecutor executor;
  //executor.add_node(node);
  //std::thread spinner = std::thread([&executor]() { executor.spin(); });

  
  // Create a ROS logger
  auto const logger = rclcpp::get_logger("hello_moveit");

  // Next step goes here
  // Create the MoveIt MoveGroup Interface
using moveit::planning_interface::MoveGroupInterface;
auto move_group_interface = MoveGroupInterface(node, "ur3_arm_pg");

//This does execute. It plans in the cartesian space. We can take the final value of the joints in the joint space.
/*
for(i=1;i<5;i++){
  
//geometry_msgs::msg::Pose msg; */
// Set a target Pose
auto const target_pose = []{
/*
  double x_val,y_val,z_val;
  double z_val_ee = -0.061;
  double z_valt = z_val_ee+0.1;
  double y_valt = 0.546;
  double x_valt = 0.018;
  //camera wrt base:
  double x_valc = x_valt;
  double y_valc = y_valt+0.065;
  double z_valc = z_valt-0.05;
  //point wrt camera: x: 0.2135, y: -0.0783, z: 0.21
  //x: 0.09, y: 0.025
  //x: 0.04 left, y: 0.09 backward, z: 0.25
  //x: 0.025 right, y: 0.03 forward, z: 0.25
  //point wrt base
  double x_valp = x_valc+0.025;
  double y_valp = y_valc;//-0.09;
  double z_valp = z_valc-0.25+0.24;
*/
  geometry_msgs::msg::Pose msg;
  /*if(i==0){*/
  
  msg.orientation.w = 1.0;
  msg.position.x = 0.28;
  msg.position.y = -0.2;
  msg.position.z = 0.5;
/*
  //camera_to_tool: y:6.5 cm forward increases, z: 5 cm down increases
  //marker_from_camera: x:7cm towards marker (tool's left) decreases, y:9.9 cm, z: 29.6 cm
  x_val = 0.042-0.07;
  y_val = 0.415+0.065+0.099;
  z_val = 0.369-0.189-0.12+0.05+0.296;
  quaternion = Eigen::AngleAxisd(0.043, Eigen::Vector3d::UnitX())
               * Eigen::AngleAxisd(3.164, Eigen::Vector3d::UnitY())
               * Eigen::AngleAxisd(-0.117, Eigen::Vector3d::UnitZ());
  std::cout << "Quaternion: " << quaternion.coeffs().transpose() << std::endl;
  msg.orientation.x = quaternion.coeffs().transpose()[0];
  msg.orientation.y = quaternion.coeffs().transpose()[1];
  msg.orientation.z = quaternion.coeffs().transpose()[2];
  msg.orientation.w = quaternion.coeffs().transpose()[3];
  msg.position.x = -0.08;
  msg.position.y = 0.43;
  msg.position.z = -0.029;//(-583+369+185) needs to go little more lower. See how z value is calculated and how to get the correct height to move there.
  //See how to calculate the correct x,y,z values of a point from the tool0 given its distance from the camera.
  std::cout << x_val << " " << y_val << " " << z_val << std::endl;
  }
  //0.707107, 0.000472, 0.194250, 0.694150
  //Normal: 0.213450 0.194259 0.310203 -0.000002 0.707107 0.707107 -0.000012
  //Standing_up: 0.000485 0.194250 0.694150 -0.707107 0.000528 0.000464 0.707107
  //Default: 0.456889 0.194252 0.066538 -0.000015 0.707107 0.707107 -0.000038
  
  //shoulder_lift joint: 0.5 to 2.96 approximately
  if(i==1){
  //tool0 from end-effector
  double z_val_ee = -0.061;
  double z_valt = z_val_ee+0.1;
  double y_valt = 0.546;
  double x_valt = 0.018;
  //camera wrt base:
  double x_valc = x_valt;
  double y_valc = y_valt+0.065;
  double z_valc = z_valt-0.05;
  //point wrt camera: x: 0.2135, y: -0.0783, z: 0.21
  //x: 0.09, y: 0.025
  //x: 0.04 left, y: 0.09 backward, z: 0.25
  //x: 0.025 right, y: 0.03 forward, z: 0.25
  //point wrt base
  double x_valp = x_valc+0.025;
  double y_valp = y_valc;//-0.09;
  double z_valp = z_valc-0.25+0.24;
  
  quaternion = Eigen::AngleAxisd(2.3, Eigen::Vector3d::UnitX())
               * Eigen::AngleAxisd(2.24, Eigen::Vector3d::UnitY())
               * Eigen::AngleAxisd(-0.057, Eigen::Vector3d::UnitZ());
  std::cout << "Quaternion: " << quaternion.coeffs().transpose() << std::endl;
  msg.orientation.x = 1.0;
  msg.orientation.y = 0.0;
  msg.orientation.z = 0.0;
  msg.orientation.w = 0.0;
  msg.position.x = x_valp;
  msg.position.y = y_valp-0.09;
  msg.position.z = z_valp;
  }
  if(i==2){
  
  msg.orientation.x = -0.707107;
  msg.orientation.y = 0.000528;
  msg.orientation.z = 0.000464;
  msg.orientation.w = 0.707107;
  msg.position.x = 0.000485;
  msg.position.y = 0.194250;
  msg.position.z = 0.694150;
  
  msg.orientation.x = 1.0;
  msg.orientation.y = 0.0;
  msg.orientation.z = 0.0;
  msg.orientation.w = 0.0;
  msg.position.x = x_valp;
  msg.position.y = y_valp-0.11;
  msg.position.z = z_valp;
  }
  if(i==3){
  
  msg.orientation.x = -0.707107;
  msg.orientation.y = 0.000528;
  msg.orientation.z = 0.000464;
  msg.orientation.w = 0.707107;
  msg.position.x = 0.000485;
  msg.position.y = 0.194250;
  msg.position.z = 0.694150;
  
  msg.orientation.x = 1.0;
  msg.orientation.y = 0.0;
  msg.orientation.z = 0.0;
  msg.orientation.w = 0.0;
  msg.position.x = x_valp;
  msg.position.y = y_valp-0.13;
  msg.position.z = z_valp;
  }
  if(i==4){
  
  msg.orientation.x = -0.707107;
  msg.orientation.y = 0.000528;
  msg.orientation.z = 0.000464;
  msg.orientation.w = 0.707107;
  msg.position.x = 0.000485;
  msg.position.y = 0.194250;
  msg.position.z = 0.694150;
  
  msg.orientation.x = 1.0;
  msg.orientation.y = 0.0;
  msg.orientation.z = 0.0;
  msg.orientation.w = 0.0;
  msg.position.x = x_valp;
  msg.position.y = y_valp-0.15;
  msg.position.z = z_valp;
  }
*/
  return msg;
}();
//const geometry_msgs::msg::Pose msg1 = msg;
move_group_interface.setPoseTarget(target_pose);
move_group_interface.setGoalTolerance(0.01);
//setPoseTarget
// Create a plan to that target pose
auto const [success, plan] = [&move_group_interface]{
  moveit::planning_interface::MoveGroupInterface::Plan msg;
  auto const ok = static_cast<bool>(move_group_interface.plan(msg));
  return std::make_pair(ok, msg);
}();

// Execute the plan
if(success) {
  move_group_interface.execute(plan);
} else {
  RCLCPP_ERROR(logger, "Planning failed!");
}
RCLCPP_ERROR(logger, "Waiting");
sleep(5);

}

/*
// This does execute. It plans in the joint space and executes. We need to run the launch file for this. The approximate one needs all the position and orientation values for each point. 
for(i=0;i<4;i++){
  double x_val,y_val,z_val;
  double z_val_ee = -0.061;
  double z_valt = z_val_ee+0.1;
  double y_valt = 0.546;
  double x_valt = 0.018;
  //camera wrt base:
  double x_valc = x_valt;
  double y_valc = y_valt+0.065;
  double z_valc = z_valt-0.05;
  //point wrt camera: x: 0.2135, y: -0.0783, z: 0.21
  //x: 0.09, y: 0.025
  //x: 0.04 left, y: 0.09 backward, z: 0.25
  //x: 0.025 right, y: 0.03 forward, z: 0.25
  //point wrt base
  double x_valp = x_valc+0.025;
  double y_valp = y_valc;//-0.09;
  double z_valp = z_valc-0.25+0.24;
  geometry_msgs::msg::Pose msg;
// Set a target Pose
//auto const target_pose = []{
  //geometry_msgs::msg::Pose msg;
  if(i==0){
  /*
  msg.orientation.x = 3.126;
  msg.orientation.y = 0.0;
  msg.orientation.z = -0.056;
  msg.orientation.w = 1.0;
  msg.position.x = 0.09;
  msg.position.y = 0.45;
  msg.position.z = 0.36;
  
  msg.orientation.x = 1.0;
  msg.orientation.y = 0.0;
  msg.orientation.z = 0.0;
  msg.orientation.w = 0.0;
  msg.position.x = x_valp;
  msg.position.y = y_valp-0.09;
  msg.position.z = z_valp;
  
  }
  //0.707107, 0.000472, 0.194250, 0.694150
  //Normal: 0.213450 0.194259 0.310203 -0.000002 0.707107 0.707107 -0.000012
  //Standing_up: 0.000485 0.194250 0.694150 -0.707107 0.000528 0.000464 0.707107
  //Default: 0.456889 0.194252 0.066538 -0.000015 0.707107 0.707107 -0.000038
  if(i==1){
  msg.orientation.x = 1.0;
  msg.orientation.y = 0.0;
  msg.orientation.z = 0.0;
  msg.orientation.w = 0.0;
  msg.position.x = x_valp;
  msg.position.y = y_valp-0.11;
  msg.position.z = z_valp;
  }
  if(i==2){
  msg.orientation.x = 1.0;
  msg.orientation.y = 0.0;
  msg.orientation.z = 0.0;
  msg.orientation.w = 0.0;
  msg.position.x = x_valp;
  msg.position.y = y_valp-0.13;
  msg.position.z = z_valp;
  }
  if(i==3){
  msg.orientation.x = 1.0;
  msg.orientation.y = 0.0;
  msg.orientation.z = 0.0;
  msg.orientation.w = 0.0;
  msg.position.x = x_valp;
  msg.position.y = y_valp-0.15;
  msg.position.z = z_valp;
  }
  //return msg;
//}();
const geometry_msgs::msg::Pose msg1 = msg;
move_group_interface.setApproximateJointValueTarget(msg1,end_effector_link.c_str());
move_group_interface.setGoalTolerance(0.01);
//setPoseTarget
// Create a plan to that target pose
auto const [success, plan] = [&move_group_interface]{
  moveit::planning_interface::MoveGroupInterface::Plan msg1;
  auto const ok = static_cast<bool>(move_group_interface.plan(msg1));
  return std::make_pair(ok, msg1);
}();

// Execute the plan
if(success) {
  move_group_interface.execute(plan);
} else {
  RCLCPP_ERROR(logger, "Planning failed!");
}
RCLCPP_ERROR(logger, "Waiting");
sleep(5);

}

  // Shutdown ROS
  rclcpp::shutdown();
  //node.reset();
  return 0;
}
*/
