#include <ignition/math/Pose3.hh>
#include "gazebo/physics/physics.hh"
#include "gazebo/common/common.hh"
#include "gazebo/gazebo.hh"
#include "ros/ros.h"
#include <thread>
#include "ros/ros.h"
#include "ros/callback_queue.h"
#include "ros/subscribe_options.h"
#include "nav_msgs/OccupancyGrid.h"
namespace gazebo
{
class Factory : public WorldPlugin
{

public: void OnRosMsg(const nav_msgs::OccupancyGridConstPtr &msg)
{
        std::cout << msg->info.width <<" " << msg->info.height<< std::endl;
        ROS_WARN("the map is loaded by gazebo.");
  //this->SetVelocity(_msg->data);
}


  public: void Load(physics::WorldPtr _parent, sdf::ElementPtr /*_sdf*/)
  {
    parent = _parent;
     this->rosNode= new ros::NodeHandle;


//    this->rosQueueThread = boost::thread(
//         boost::bind(&Factory::QueueThread, this));

        ros::SubscribeOptions so = ros::SubscribeOptions::create<nav_msgs::OccupancyGrid>(
             "/map",1,
          boost::bind( &Factory::map_cb,this,_1),
        ros::VoidPtr(), &this->rosQueue);
           this->map_sub = this->rosNode->subscribe(so);

        // Custom Callback Queue
        this->rosQueueThread = boost::thread( boost::bind( &Factory::QueueThread,this ) );

    ROS_WARN("obstacle map initialize");






  }
private: void map_cb(const nav_msgs::OccupancyGrid::ConstPtr& msg){
  obstacle_map = *msg;
  int cur_x = 0, cur_y=-1;
//  std::cout <<obstacle_map.info.width << " " << obstacle_map.info.height<<std::endl;
   bit_map.resize(obstacle_map.info.width);
   for(unsigned int i=0; i<obstacle_map.info.width ;i++){
       bit_map[i].resize(obstacle_map.info.height);
   }
  if(obstacle_map.data.size()>0){
      std::cout << "size" <<obstacle_map.data.size()  <<std::endl;

      for(unsigned int i=0;i< obstacle_map.data.size();i++){

          //split 1-d data to 2-D
          if( i% ( obstacle_map.info.width) ==0){
              cur_y++;
              cur_x=0;
          }else{
              cur_x++;
          }
          if(obstacle_map.data[i]!=0){
           bit_map[cur_x][cur_y] = true;
          }else{
            bit_map[cur_x][cur_y] = false;

          }
      }
      for(unsigned int i=0;i< obstacle_map.info.height;i++){
          for(unsigned int j=0;j<obstacle_map.info.width;j++){
              std::cout <<  bit_map[j][i] ;
          }
          std::cout << std::endl;
      }
  }

    //create obstacle

    for(unsigned int i=0;i<obstacle_map.info.width;i++){

        for(unsigned int j=0;j< obstacle_map.info.height ;j++){
            if(bit_map[i][j]){
                sdf::SDF sphereSDF;
                char a[50];
                char b[50];
                sprintf(a , "<pose>%d %d %d 0 0 0</pose>", i,j,0.75);
                std::string pose_str = std::string(a);

                std::string sdf_str(  "<sdf version ='1.4'>\
                                    <model name ='sphere'> \
                        <static>true</static>"+
                                    pose_str+
                                   "<link name ='link'>\
                                     <pose>0 0 0 0 0 0</pose>\
                                     <visual name ='visual'>\
                                       <geometry>\
                                         <box> <size>1.0 1.0 1.5</size></box>\
                                       </geometry>\
                                     </visual>\
                                   </link>\
                                 </model>\
                               </sdf>");
                sphereSDF.SetFromString(sdf_str);

                sdf::ElementPtr model = sphereSDF.Root()->GetElement("model");

                sprintf(b,"box_%d",i*100+j);
                std::string model_name(b) ;
                model->GetAttribute("name")->SetFromString(model_name);
                //_parent->InsertModelSDF(sphereSDF);
               // parent->InsertModelSDF(sphereSDF);

            }
        }
  }
}


private: void QueueThread()
{
  static const double timeout = 0.01;
  while (this->rosNode->ok())
  {
    this->rosQueue.callAvailable(ros::WallDuration(timeout));
  }
}
private:    ros::NodeHandle* rosNode;
    private: ros::CallbackQueue rosQueue;
    private: boost::thread rosQueueThread;
private: nav_msgs::OccupancyGrid obstacle_map;
    private: ros::Subscriber map_sub;
private:    std::vector<std::vector<bool>> bit_map;
private: physics::WorldPtr  parent;
};

// Register this plugin with the simulator
GZ_REGISTER_WORLD_PLUGIN(Factory)
}