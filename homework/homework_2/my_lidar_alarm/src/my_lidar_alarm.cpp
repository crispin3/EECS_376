// wsn example program to illustrate LIDAR processing.  1/23/15
// Program modified by Jonathan Crispin for Homework 2 in EECS 376 mobile robotics for 2/8/2017

#include <ros/ros.h> //Must include this for all ROS cpp projects
#include <sensor_msgs/LaserScan.h>
#include <std_msgs/Float32.h> //Including the Float32 class from std_msgs
#include <std_msgs/Bool.h> // boolean message 


const double MIN_SAFE_DISTANCE = 1.0; // set alarm if anything is within 0.5m of the front of robot

// these values to be set within the laser callback
float ping_dist_in_front_=5.0; // global var to hold length of a SINGLE LIDAR ping--in front
int ping_index_= -1; // NOT real; callback will have to find this
double angle_min_=0.0;
double angle_max_=0.0;
double angle_increment_=1.0;
double range_min_ = 0.0;
double range_max_ = 0.0;
bool laser_alarm_=false;
double a_min = 0.0;
double a_max = 0.0;
double i = 0.0;

ros::Publisher lidar_alarm_publisher_;
ros::Publisher lidar_dist_publisher_;
// really, do NOT want to depend on a single ping.  Should consider a subset of pings
// to improve reliability and avoid false alarms or failure to see an obstacle

void laserCallback(const sensor_msgs::LaserScan& laser_scan) {
    if (ping_index_<0)  {
        //for first message received, set up the desired index of LIDAR range to eval
        angle_min_ = laser_scan.angle_min;
        angle_max_ = laser_scan.angle_max;
        angle_increment_ = laser_scan.angle_increment;
        range_min_ = laser_scan.range_min;
        range_max_ = laser_scan.range_max;
        // what is the index of the ping that is straight ahead?
        // BETTER would be to use transforms, which would reference how the LIDAR is mounted;
        // but this will do for simple illustration
        ping_index_ = (int) ((0.0 -angle_min_)/angle_increment_);
	a_min = ((-1.2 - angle_min_) / angle_increment_); // Added by Jonathan, minimum angle between -60 to -90 degrees
	a_max = ((1.2 - angle_min_) / angle_increment_); // Added by Jonathan, maximum angle between 60 to 90 degrees
        
    }

// Lines added between 48-49 by Jonathan to go through a range of angles by angle_increment_ between a_min and a_max
   for (i = a_min;i < a_max;angle_increment_) {
   i = i + angle_increment_;  
   ping_dist_in_front_ = laser_scan.ranges[i]; // Modified by Jonathan, laser_scan.ranges uses i
   ROS_INFO("ping dist in front = %f",ping_dist_in_front_);
   if (ping_dist_in_front_<MIN_SAFE_DISTANCE) {
       ROS_WARN("COMING IN TOO HOT!!!");
       laser_alarm_=true;
   }
   else {
       laser_alarm_=false;
   }
   std_msgs::Bool lidar_alarm_msg;
   lidar_alarm_msg.data = laser_alarm_;
   lidar_alarm_publisher_.publish(lidar_alarm_msg);
   std_msgs::Float32 lidar_dist_msg;
   lidar_dist_msg.data = ping_dist_in_front_;
   lidar_dist_publisher_.publish(lidar_dist_msg);   
}
}

int main(int argc, char **argv) {
    ros::init(argc, argv, "lidar_alarm"); //name this node
    ros::NodeHandle nh; 
    //create a Subscriber object and have it subscribe to the lidar topic
    ros::Publisher pub = nh.advertise<std_msgs::Bool>("lidar_alarm", 1);
    lidar_alarm_publisher_ = pub; // let's make this global, so callback can use it
    ros::Publisher pub2 = nh.advertise<std_msgs::Float32>("lidar_dist", 1);  
    lidar_dist_publisher_ = pub2;
    ros::Subscriber lidar_subscriber = nh.subscribe("robot0/laser_0", 1, laserCallback);
    ros::spin(); //this is essentially a "while(1)" statement, except it
    // forces refreshing wakeups upon new data arrival
    // main program essentially hangs here, but it must stay alive to keep the callback function alive
    return 0; // should never get here, unless roscore dies
}

