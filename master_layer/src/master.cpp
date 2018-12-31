#include <ros/ros.h>

#include <gate.h>
#include <single_buoy.h>
#include <torpedo.h>

#include <straight_server.h>
#include <move_sideward_server.h>
#include <move_forward_server.h>
#include <move_downward_server.h>

#include <actionlib/client/simple_action_client.h>

#include <motion_layer/anglePIDAction.h>
#include <motion_layer/rollPIDAction.h>
#include <motion_layer/pitchPIDAction.h>
#include <motion_layer/forwardPIDAction.h>
#include <motion_layer/sidewardPIDAction.h>
#include <motion_layer/upwardPIDAction.h>

#include <actionlib/client/terminal_state.h>
#include <std_msgs/String.h>
#include <task_handler.h>
#include <boost/thread.hpp>

using namespace std;

void spinThread() {
    ROS_INFO("Spinning");
    ros::spin();
}

int main(int argc, char** argv) {
    ros::init(argc, argv, "master");
    ros::NodeHandle nh;
    ros::Publisher task_pub = nh.advertise<std_msgs::String>("/current_task", 1); 
    ros::Time::init();
    int pub_count = 0;
    std_msgs::String current_task;
    ros::Rate loop_rate(10);
    taskHandler th(15); // time out 15 seconds
    singleBuoy single_buoy;

    boost::thread spin_thread(&spinThread);
   
    /////////////////////////////////////////////

    current_task.data = "red_buoy";
    while (ros::ok() && pub_count <= 5) {
        task_pub.publish(current_task);
        pub_count++;
        loop_rate.sleep();
    }
    pub_count = 0;
    nh.setParam("/current_task", "red_buoy");
    ROS_INFO("Current task: Red Buoy");
    
    single_buoy.setActive(true); // blocking function, will terminalte after completion 
    single_buoy.setActive(false);
    
    ROS_INFO("Completed the first buoy task, Now let's move on to the second");

    ////////////////////////////////////////

    current_task.data = "yellow_buoy";
    while (ros::ok() && pub_count <= 5) {
        task_pub.publish(current_task);
        pub_count++;
        loop_rate.sleep();
    }
    pub_count = 0;
    nh.setParam("/current_task", "yellow_buoy");
    ROS_INFO("Current task: Yellow Buoy");

    moveSideward move_sideward(-100);
    move_sideward.setActive(true);
    ROS_INFO("Finding Yellow Buoy....");
    th.isDetected("yellow_buoy", 15); // time out of 15 seconds
    ROS_INFO("Yellow Buoy Detected");
    move_sideward.setActive(false);

    single_buoy.setActive(true);
    single_buoy.setActive(false);

    ROS_INFO("Yellow Buoy done");
  
    //////////////////////////////////////////////

    current_task.data = "green_buoy";
    while (ros::ok() && pub_count <= 5) {
        task_pub.publish(current_task);
        pub_count++;
        loop_rate.sleep();
    }
    pub_count = 0;
    nh.setParam("/current_task", "green_buoy");
    ROS_INFO("Current task: Green Buoy");

    moveDownward move_downward(-50); // for real world
    move_downward.setActive(true);
    ros::Duration(4).sleep();
    move_downward.setActive(false);

    while (ros::ok() && pub_count <= 5) { // for gazebo
        nh.setParam("/pwm_heave", -50);
        pub_count++;
        loop_rate.sleep();
    }
    pub_count = 0;
    
    ROS_INFO("Green Buoy Task, Going Down");
    ros::Duration(6).sleep();
    ROS_INFO("Green Buoy Task, At bottom");
    nh.setParam("/pwm_heave", 0);

    // moveSideward move_sideward(100);
    move_sideward.setThrust(100);
    move_sideward.setActive(true); // until the green buoy is detected (for vision node)
    
    // ros::Duration(10).sleep(); // should move approx. 10m straight sideways
    
    th.isDetected("green_buoy", 25); // time out of 15 seconds
    ROS_INFO("Green Buoy Detected");
    move_sideward.setActive(false);

    single_buoy.setActive(true); // blocking function, will terminalte after completion
    single_buoy.setActive(false);

    ROS_INFO("Green Buoy Done");
  
    ////////////////////////////////////////////////

    // Example for sideward, forward, and angle PIDs

    // actionlib::SimpleActionClient<motion_layer::sidewardPIDAction> sidewardPIDClient("sidewardPID");
    // motion_layer::sidewardPIDGoal sidewardPIDgoal;

    // ROS_INFO("Waiting for sidewardPID server to start, Buoy-Gate transition.");
    // sidewardPIDClient.waitForServer();

    // ROS_INFO("sidewardPID server started, sending goal, Buoy-Gate transition.");
    // sidewardPIDgoal.target_distance = 0;
    // sidewardPIDClient.sendGoal(sidewardPIDgoal);

    // actionlib::SimpleActionClient<motion_layer::forwardPIDAction> forwardPIDClient("forwardPID");
    // motion_layer::forwardPIDGoal forwardPIDgoal;

    // ROS_INFO("Waiting for forwardPID server to start, Buoy-Gate transition.");
    // forwardPIDClient.waitForServer();

    // ROS_INFO("forwardPID server started, sending goal, Buoy-Gate transition.");
    // forwardPIDgoal.target_distance = 0;
    // forwardPIDClient.sendGoal(forwardPIDgoal); // task_handler node

    //////////////////////////////////////////////////

    current_task.data = "gate";
    while (ros::ok() && pub_count <= 5) {
        task_pub.publish(current_task);
        pub_count++;
        loop_rate.sleep();
    }
    pub_count = 0;
    nh.setParam("/current_task", "gate");
    ROS_INFO("Current task: Gate");

    ROS_INFO("Gate Task, going down");
    move_downward.setActive(true);
    ros::Duration(4).sleep();
    move_downward.setActive(false);
    ROS_INFO("Gate Task, at the bottom");

    move_sideward.setThrust(-100);
    move_sideward.setActive(true); // until gate is detected
    
    // ros::Duration(5).sleep(); // time depends, better to have a node telling when the gate is detected

    th.isDetected("gate", 10);

    move_sideward.setActive(false);

    gateTask gate_task;
    gate_task.setActive(true); // blocking function, will terminalte after completion
    gate_task.setActive(false);

    ///////////////////////////////////////////////////

    // Gate-Torpedo Transition

    current_task.data = "red_torpedo";
    while (ros::ok() && pub_count <= 5) {
        task_pub.publish(current_task);
        pub_count++;
        loop_rate.sleep();
    }
    pub_count = 0;
    nh.setParam("/current_task", "red_torpedo");
    ROS_INFO("Current task: Red Torpedo");

    actionlib::SimpleActionClient<motion_layer::anglePIDAction> anglePIDClient("turnPID");
    motion_layer::anglePIDGoal anglePIDGoal;

    ROS_INFO("Waiting for anglePID server to start.");
    anglePIDClient.waitForServer();

    ROS_INFO("anglePID server started, sending goal.");

    anglePIDGoal.target_angle = 60;
    anglePIDClient.sendGoal(anglePIDGoal);

    th.isAchieved(60, 2, "angle");

    th.isDetected("red_torpedo", 5);

    ///////////////////////////////////////////////////

    Torpedo torpedo;

    torpedo.setActive(true);
    torpedo.setActive(false);

    ////////////////////////////////////////////////////

    current_task.data = "green_torpedo";
    while (ros::ok() && pub_count <= 5) {
        task_pub.publish(current_task);
        pub_count++;
        loop_rate.sleep();
    }
    pub_count = 0;
    nh.setParam("/current_task", "green_torpedo");
    ROS_INFO("Current task: Green Torpedo");

    move_sideward.setThrust(100);
    move_sideward.setActive(true);

    th.isDetected("green_torpedo", 5);

    move_sideward.setActive(false);

    torpedo.setActive(true);
    torpedo.setActive(false);

    /////////////////////////////////////////////////////

    // Torpedo-MarkerDropper Transition

    /////////////////////////////////////////////////////

    // MarkerDropper

    /////////////////////////////////////////////////////

    // Octagon

    /////////////////////////////////////////////////////
    
    nh.setParam("/kill_signal", true);
    nh.setParam("/kill_signal", false);

    spin_thread.join();

    return 0;
}
