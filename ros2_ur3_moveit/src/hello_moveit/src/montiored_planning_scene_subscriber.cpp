#include <memory>
#include <stdio.h>
#include <string>
#include <fstream>
#include <cstdlib>
#include "rclcpp/rclcpp.hpp"
#include "moveit_msgs/msg/planning_scene.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "moveit_msgs/msg/robot_state.hpp"
#include "moveit_msgs/action/execute_trajectory.hpp"
using std::placeholders::_1;
float position0[600]; 
int i=0;
static std::fstream fout;
static std::string state;
int motion_fin=0;

class MinimalSubscriber1 : public rclcpp::Node
{
  public:
    MinimalSubscriber1()
    : Node("minimal_subscriber_planning_scene")
    {
      subscription_ = this->create_subscription<moveit_msgs::msg::PlanningScene>(      "/monitored_planning_scene", 1000, std::bind(&MinimalSubscriber1::topic_callback, this, _1));
      subscription1_ = this->create_subscription<moveit_msgs::action::ExecuteTrajectory_FeedbackMessage>(      "/execute_trajectory/_action/feedback", 1000, std::bind(&MinimalSubscriber1::feedback_callback, this, _1));
      //subscription_=this->create_subscription<moveit_msgs::msg::PlanningScene>(            "/monitored_planning_scene", 1000, [this](const moveit_msgs::msg::PlanningScene::SharedPtr msg) {topic_callback(msg, nullptr);});
      //subscription1_ = this->create_subscription<moveit_msgs::action::ExecuteTrajectory_FeedbackMessage>("/execute_trajectory/_action/feedback", 1000, [this](const moveit_msgs::action::ExecuteTrajectory_FeedbackMessage::SharedPtr execute_status) {topic_callback(nullptr, execute_status);});
     }
  private:
    void feedback_callback(const moveit_msgs::action::ExecuteTrajectory_FeedbackMessage::SharedPtr execute_status) const
    {
      const auto feedback = execute_status->feedback;
      state = feedback.state;
    }
      
    void topic_callback(const moveit_msgs::msg::PlanningScene::SharedPtr msg) const
    {
      std::string name = msg->name;
      //RCLCPP_INFO(this->get_logger(), "I heard: %s", name.c_str());
      const auto rs = std::make_shared<moveit_msgs::msg::RobotState>(msg->robot_state);
      const auto js = rs->joint_state;
      //RCLCPP_INFO(this->get_logger(), "I heard: '%d'", i);
      //RCLCPP_INFO(this->get_logger(), "I heard: '%lf'", js.position[0]);  
       fout<<js.position[0]<<","<<js.position[1]<<","<<js.position[2]<<","<<js.position[3]<<","      <<js.position[4]<<","<<js.position[5]<<"\n";
      
      position0[i] = js.position[0];
      i=i+1;
      if(motion_fin==1){
      	RCLCPP_INFO(this->get_logger(), "Waiting\n");
        //sleep(5);        
        motion_fin=0;
      }
      
      if (strcmp(state.c_str(),"IDLE")==0){
      	fout<<"\n";
      	motion_fin=1;
      }
      /*
      (const moveit_msgs::msg::RobotState::SharedPtr) const robot_state1 = msg.robot_state;
      const sensor_msgs::msg::JointState::SharedPtr joint_state1 = robot_state1.joint_state;
      position0[i] = joint_state1->position[0];
      i=i+1;
    }
    */
    }
    rclcpp::Subscription<moveit_msgs::msg::PlanningScene>::SharedPtr subscription_;
    rclcpp::Subscription<moveit_msgs::action::ExecuteTrajectory_FeedbackMessage>::SharedPtr subscription1_;
};
//->data.c_str()
int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  fout.open("/home/laasya/joint_position_values.csv", std::ios::out /*| std::ios::app*/);
  rclcpp::spin(std::make_shared<MinimalSubscriber1>());
  for (int j=0;j<250;j++){
  printf("Value at %d: %lf\n",j,position0[j]);
  }
  fout.close();
  rclcpp::shutdown();
  return 0;
}
