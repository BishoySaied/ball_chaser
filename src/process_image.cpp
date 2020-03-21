#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
    ball_chaser::DriveToTarget  srv;
	srv.request.linear_x = lin_x;
	srv.request.angular_z = ang_z;
	    // Call the safe_move service and pass the requested joint angles
    if (!client.call(srv))
        ROS_ERROR("Failed to call service ball_chaser");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;

/////////////////////////
    bool is_white_pixel = false;
	uint32_t height_white_pixel = 0;
	uint64_t position_white_pixel = 0;
	uint64_t region = (uint64_t)(img.step/3); //approx
			ROS_INFO("region num of pixels %1.2f" , region  );
			ROS_INFO("one step size = %d " , img.step  );
//region num of pixels 640000
//White Pixel is detected at step = 917157
//[ INFO] [1584785806.971849003, 523.146000000]: The requested wheel velocities - X: 0.0500000 , Z: 0.000000
//[ INFO] [1584785806.973016580, 523.146000000]: command forward

    // Loop through each pixel in the image and check if its equal to the first one
    for (uint64_t i = 0; i < img.height * img.step; i+=3) {
		
        if (img.data[i] == white_pixel && img.data[i+1] == white_pixel && img.data[i+2] == white_pixel) {
            is_white_pixel = true;
			height_white_pixel = img.height ;
			position_white_pixel = (uint64_t) (i % img.step); //capture current position w.r.t the step size
			ROS_INFO("White Pixel is detected at step = %d  Height = %d  " , position_white_pixel  , height_white_pixel);
            break;
        }
		
    }

    // If the image is uniform and the arm is not moving, move the arm to the center
    if (is_white_pixel == false){
		//stop
				drive_robot(0, 0);
				ROS_INFO("command stop" );

	}else {
		//categorize step of white pixel place
		if (position_white_pixel >= (2*region))
		{//righ
		drive_robot(0, -0.05);
						ROS_INFO("command right" );


	}
	else{
		if (position_white_pixel >= region){
			//forward
							drive_robot(0.05, 0);
											ROS_INFO("command forward" );


		}
		else{
			//left
				drive_robot(0, 0.05);
								ROS_INFO("command left" );


		}
	}
		
	}
       


}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}